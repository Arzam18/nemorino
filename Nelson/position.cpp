/*
This file is part of Nemorino.

Nemorino is free software : you can redistribute it and /or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Nemorino is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Nemorino.If not, see < http://www.gnu.org/licenses/>.
*/

#include <sstream>
#include <string>
#include <cstring>
#include <iomanip>
#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <ctype.h>
#include <regex>
#include <cstddef>
#include <algorithm>
#include "position.h"
#include "material.h"
#include "settings.h"
#include "hashtables.h"
#include "evaluation.h"
#include "tbprobe.h"

nnue::Network network;

constexpr std::string_view PieceToChar("QqRrBbNnPpKk ");

Position::Position()
{
	setFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

Position::Position(std::string fen)
{
	setFromFEN(fen);
}

Position::Position(uint8_t* data)
{
	unpack(data);
}

Position::Position(Position& pos) {
	//These are the members, which are copied by the copy constructor and contain the full information
	this->OccupiedByColor.at(0) = pos.OccupiedByColor.at(0);
	this->OccupiedByColor.at(1) = pos.OccupiedByColor.at(1);

	//for (int i = 0; i < 6; ++i) this->OccupiedByPieceType.at(i) = pos.OccupiedByPieceType.at(i);
	this->OccupiedByPieceType = pos.OccupiedByPieceType;
	this->Hash = pos.Hash;
	this->MaterialKey = pos.MaterialKey;
	this->PawnKey = pos.PawnKey;
	this->EPSquare = pos.EPSquare;
	this->CastlingOptions = pos.CastlingOptions;
	this->DrawPlyCount = pos.DrawPlyCount;
	this->SideToMove = pos.SideToMove;
	this->pliesFromRoot = pos.pliesFromRoot;
	//for (int i = 0; i < 64; ++i) this->Board.at(i) = pos.Board.at(i);
	this->Board = pos.Board;
	this->PsqEval = pos.PsqEval;
	//King Squares
	this->kingSquares.at(0) = pos.kingSquares.at(0);
	this->kingSquares.at(1) = pos.kingSquares.at(1);
	this->material = pos.material;
	this->pawn = pos.pawn;
	this->attacks = pos.attacks;
	this->attackedByThem = pos.attackedByThem;
	this->attackedByUs = pos.attackedByUs;
	this->attacksByPt = pos.attacksByPt;
	if (settings::parameter.UseNNUE) {
		std::memcpy(&nnInput, &pos.nnInput, sizeof(nnInput));
		nnInput.set = false;
	}
	previous = &pos;
}

Position::~Position()
{

}

void Position::copy(const Position& pos) noexcept {
	this->attacks = pos.attacks;
	this->attacksByPt = pos.attacksByPt;
	this->attackedByUs = pos.attackedByUs;
	this->attackedByThem = pos.attackedByThem;
	this->lastAppliedMove = pos.lastAppliedMove;
	this->StaticEval = pos.StaticEval;
	this->bbPinned.at(static_cast<int>(Color::WHITE)) = pos.bbPinned.at(static_cast<int>(Color::WHITE));
	this->bbPinned.at(static_cast<int>(Color::BLACK)) = pos.bbPinned.at(static_cast<int>(Color::BLACK));
	this->bbPinner.at(static_cast<int>(Color::WHITE)) = pos.bbPinner.at(static_cast<int>(Color::WHITE));
	this->bbPinner.at(static_cast<int>(Color::BLACK)) = pos.bbPinner.at(static_cast<int>(Color::BLACK));

}

int Position::AppliedMovesBeforeRoot = 0;

bool Position::ApplyMove(Move move) {
	pliesFromRoot++;
	const Square fromSquare = from(move);
	Square toSquare = to(move);
	const Piece moving = Board.at(fromSquare);
	capturedInLastMove = Board.at(toSquare);
	remove(fromSquare);
	const MoveType moveType = type(move);
	Piece convertedTo = Piece::BLANK;

	if (moving >= Piece::WKING) {
		nnInput.deltas.at(0).square = 64;
	}
	else {
		nnInput.deltas.at(0).square = fromSquare;
		nnInput.deltas.at(0).piece_before = TransformPiece(moving);
		nnInput.deltas.at(1).square = toSquare;
		nnInput.deltas.at(1).piece_before = TransformPiece(capturedInLastMove);
		nnInput.deltas.at(2).square = 64;
	}

	switch (moveType) {
	case MoveType::NORMAL:
		set<false>(moving, toSquare);
		//update epField
		if ((GetPieceType(moving) == PAWN)
			&& (abs(static_cast<int>(toSquare) - static_cast<int>(fromSquare)) == 16)
			&& (GetEPAttackersForToField(toSquare) & PieceBB(PAWN, ~SideToMove)))
		{
			SetEPSquare(static_cast<Square>(toSquare - PawnStep()));
		}
		else
			SetEPSquare(OUTSIDE);
		if (GetPieceType(moving) == PAWN || capturedInLastMove != BLANK) {
			DrawPlyCount = 0;
			if (GetPieceType(moving) == PAWN) {
				PawnKey ^= (ZobristKeys.at(moving).at(fromSquare));
				PawnKey ^= (ZobristKeys.at(moving).at(toSquare));
			}
			if (GetPieceType(capturedInLastMove) == PAWN) {
				PawnKey ^= ZobristKeys.at(capturedInLastMove).at(toSquare);
			}
		}
		else
			++DrawPlyCount;
		//update castlings
		updateCastleFlags(fromSquare, toSquare);
		//Adjust material key
		if (capturedInLastMove != BLANK) {
			if (MaterialKey == MATERIAL_KEY_UNUSUAL) {
				if (checkMaterialIsUnusual()) {
					material->Evaluation = calculateMaterialEval(*this);
					material->SetIsTableBaseEntry(popcount(OccupiedByColor.at(static_cast<int>(Color::WHITE)) | OccupiedByColor.at(static_cast<int>(Color::BLACK))) <= tablebases::MaxCardinality);
				}
				else MaterialKey = calculateMaterialKey();
			}
			else MaterialKey -= materialKeyFactors.at(capturedInLastMove);
			material = probe(MaterialKey);
		}
		if (kingSquares.at(static_cast<int>(SideToMove)) == fromSquare) kingSquares.at(static_cast<int>(SideToMove)) = toSquare;
		//update dirtyPiece
		break;
	case MoveType::ENPASSANT:
		set<true>(moving, toSquare);
		remove(static_cast<Square>(toSquare - PawnStep()));
		capturedInLastMove = static_cast<Piece>(static_cast<int>(BPAWN) - static_cast<int>(SideToMove));
		if (MaterialKey == MATERIAL_KEY_UNUSUAL) {
			material->Evaluation = calculateMaterialEval(*this);
		}
		else {
			MaterialKey -= materialKeyFactors.at(static_cast<int>(BPAWN) - static_cast<int>(SideToMove));
			material = probe(MaterialKey);
		}
		SetEPSquare(OUTSIDE);
		DrawPlyCount = 0;
		PawnKey ^= ZobristKeys.at(moving).at(fromSquare);
		PawnKey ^= ZobristKeys.at(moving).at(toSquare);
		PawnKey ^= ZobristKeys.at(static_cast<int>(BPAWN) - static_cast<int>(SideToMove)).at(toSquare - PawnStep());
		nnInput.deltas.at(2).square = gsl::narrow_cast<uint8_t>(toSquare - PawnStep());
		nnInput.deltas.at(2).piece_before = TransformPiece(capturedInLastMove);
		break;
	case MoveType::PROMOTION:
		convertedTo = GetPiece(promotionType(move), SideToMove);
		set<false>(convertedTo, toSquare);
		updateCastleFlags(fromSquare, toSquare);
		SetEPSquare(OUTSIDE);
		DrawPlyCount = 0;
		PawnKey ^= ZobristKeys.at(moving).at(fromSquare);
		//Adjust Material Key
		if (checkMaterialIsUnusual()) {
			MaterialKey = MATERIAL_KEY_UNUSUAL;
			material = initUnusual(*this);
		}
		else {
			if (MaterialKey == MATERIAL_KEY_UNUSUAL) MaterialKey = calculateMaterialKey();
			else
				MaterialKey = MaterialKey - materialKeyFactors.at(static_cast<int>(WPAWN) + static_cast<int>(SideToMove)) - materialKeyFactors.at(capturedInLastMove) + materialKeyFactors.at(convertedTo);
			material = probe(MaterialKey);
			material->SetIsTableBaseEntry(popcount(OccupiedByColor.at(static_cast<int>(Color::WHITE)) | OccupiedByColor.at(static_cast<int>(Color::BLACK))) <= tablebases::MaxCardinality);
		}
		break;
	case MoveType::CASTLING:
		if (toSquare == G1 + (static_cast<int>(SideToMove) *56) || (toSquare == InitialRookSquare.at(2 * static_cast<int>(SideToMove)))) {
			//short castling
			remove(InitialRookSquare.at(2 * static_cast<int>(SideToMove))); //remove rook
			toSquare = static_cast<Square>(G1 + (static_cast<int>(SideToMove) * 56));
			set<true>(moving, toSquare); //Place king
			const Square rookTo = static_cast<Square>(toSquare - 1);
			set<true>(static_cast<Piece>(static_cast<int>(WROOK) + static_cast<int>(SideToMove)),
				rookTo); //Place rook
		}
		else {
			//long castling
			remove(InitialRookSquare.at(2 * static_cast<int>(SideToMove) + 1)); //remove rook
			toSquare = static_cast<Square>(C1 + (static_cast<int>(SideToMove) * 56));
			set<true>(moving, toSquare); //Place king
			const Square rookTo = static_cast<Square>(toSquare + 1);
			set<true>(static_cast<Piece>(static_cast<int>(WROOK) + static_cast<int>(SideToMove)),
				rookTo); //Place rook
		}
		RemoveCastlingOption(static_cast<CastleFlag>(W0_0 << (2 * static_cast<int>(SideToMove))));
		RemoveCastlingOption(static_cast<CastleFlag>(W0_0_0 << (2 * static_cast<int>(SideToMove))));
		SetEPSquare(OUTSIDE);
		++DrawPlyCount;
		kingSquares.at(static_cast<int>(SideToMove)) = toSquare;
		break;
	}
	assert(kingSquares.at(static_cast<int>(Color::WHITE)) == lsb(PieceBB(KING, Color::WHITE)));
	assert(kingSquares.at(static_cast<int>(Color::BLACK)) == lsb(PieceBB(KING, Color::BLACK)));
	tt::prefetch(Hash); //for null move
	SwitchSideToMove();
	tt::prefetch(Hash);
	movepointer = 0;
	attackedByUs = calculateAttacks(SideToMove);
	//calculatePinned();
	attackedByThem = calculateAttacks(~SideToMove);
	//assert((checkMaterialIsUnusual() && MaterialKey == MATERIAL_KEY_UNUSUAL) || MaterialKey == calculateMaterialKey());
	//assert(PawnKey == calculatePawnKey());
	CalculatePinnedPieces();
	if (pawn->Key != PawnKey) pawn = pawn::probe(*this);
	lastAppliedMove = move;
	if (material->IsTheoreticalDraw()) result = Result::DRAW;
	return !(attackedByUs & PieceBB(KING, ~SideToMove));
	//if (attackedByUs & PieceBB(KING, ~SideToMove)) return false;
	//attackedByThem = calculateAttacks(Color(SideToMove ^1));
	//return true;
}

void Position::AddUnderPromotions() noexcept {
	if (canPromote) {
		movepointer--;
		const int moveCount = movepointer;
		for (int i = 0; i < moveCount; ++i) {
			const Move pmove = moves.at(i).move;
			if (type(pmove) == MoveType::PROMOTION) {
				AddMove(createMove<MoveType::PROMOTION>(from(pmove), to(pmove), KNIGHT));
				AddMove(createMove<MoveType::PROMOTION>(from(pmove), to(pmove), ROOK));
				AddMove(createMove<MoveType::PROMOTION>(from(pmove), to(pmove), BISHOP));
			}
		}
		AddNullMove();
	}
}

Move Position::NextMove() {
	if (generationPhases.at(generationPhase) == NONE) return MOVE_NONE;
	Move move = MOVE_NONE;
	do {
		processedMoveGenerationPhases |= 1 << (int)generationPhases.at(generationPhase);
		if (moveIterationPointer < 0) {
			phaseStartIndex = movepointer - (movepointer != 0);
			switch (generationPhases.at(generationPhase)) {
			case KILLER:
				moveIterationPointer = 0;
				break;
			case NON_LOOSING_CAPTURES:
				GenerateMoves<NON_LOOSING_CAPTURES>();
				evaluateByCaptureScore();
				moveIterationPointer = 0;
				break;
			case LOOSING_CAPTURES:
				GenerateMoves<LOOSING_CAPTURES>();
				evaluateByCaptureScore(phaseStartIndex);
				moveIterationPointer = 0;
				break;
				//case QUIETS:
				//	GenerateMoves<QUIETS>();
				//	evaluateByHistory(phaseStartIndex);
				//	shellSort(moves + phaseStartIndex, movepointer - phaseStartIndex - 1);
				//	moveIterationPointer = 0;
				//	break;
			case QUIETS_POSITIVE:
				GenerateMoves<QUIETS>();
				evaluateByHistory(phaseStartIndex);
				firstNegative = std::partition(moves.data() + phaseStartIndex, &moves.data()[movepointer - 1], positiveScore);
				insertionSort(moves.data() + phaseStartIndex, firstNegative);
				moveIterationPointer = 0;
				break;
			case CHECK_EVASION:
				GenerateMoves<CHECK_EVASION>();
				evaluateCheckEvasions(phaseStartIndex);
				moveIterationPointer = 0;
				break;
			case QUIET_CHECKS:
				GenerateMoves<QUIET_CHECKS>();
				//evaluateBySEE(phaseStartIndex);
				//insertionSort(moves + phaseStartIndex, moves + (movepointer - 1));
				moveIterationPointer = 0;
				break;
			case UNDERPROMOTION:
				if (canPromote) {
					movepointer--;
					const int moveCount = movepointer;
					for (int i = 0; i < moveCount; ++i) {
						const Move pmove = moves.at(i).move;
						if (type(pmove) == MoveType::PROMOTION) {
							AddMove(createMove<MoveType::PROMOTION>(from(pmove), to(pmove), KNIGHT));
							AddMove(createMove<MoveType::PROMOTION>(from(pmove), to(pmove), ROOK));
							AddMove(createMove<MoveType::PROMOTION>(from(pmove), to(pmove), BISHOP));
						}
					}
					AddNullMove();
					phaseStartIndex = moveCount;
					moveIterationPointer = 0;
					break;
				}
				else return MOVE_NONE;
			case FORKS:
				GenerateForks(true);
				moveIterationPointer = 0;
				break;
			case FORKS_NO_CHECKS:
				GenerateForks(false);
				moveIterationPointer = 0;
				break;
			case ALL:
				GenerateMoves<ALL>();
				moveIterationPointer = 0;
				break;
			default:
				break;
			}

		}
		switch (generationPhases.at(generationPhase)) {
		case HASHMOVE:
			++generationPhase;
			moveIterationPointer = -1;
			if (validateMove(hashMove)) return hashMove;
			break;
		case KILLER:
			while (killerManager && moveIterationPointer < killer::NB_KILLER) {
				const Move killerMove = killerManager->getMove(*this, moveIterationPointer);
				++moveIterationPointer;
				//if (killerMove != MOVE_NONE && validateMove(killerMove))  return killerMove;
				if (killerMove != MOVE_NONE && killerMove != hashMove && validateMove(killerMove)) return killerMove;
			}
			++generationPhase;
			moveIterationPointer = -1;
			break;
		case NON_LOOSING_CAPTURES:
			move = getBestMove(phaseStartIndex + moveIterationPointer);
			if (move) {
				++moveIterationPointer;
				goto end_post_hash;
			}
			else {
				++generationPhase;
				moveIterationPointer = -1;
			}
			break;
		case LOOSING_CAPTURES: case QUIETS_NEGATIVE:
			move = getBestMove(phaseStartIndex + moveIterationPointer);
			if (move) {
				++moveIterationPointer;
				goto end_post_killer;
			}
			else {
				++generationPhase;
				moveIterationPointer = -1;
			}
			break;
			//case QUIETS:
			//	move = moves.at(phaseStartIndex + moveIterationPointer).move;
			//	if (move) {
			//		++moveIterationPointer;
			//		goto end_post_killer;
			//	}
			//	else {
			//		++generationPhase;
			//		moveIterationPointer = -1;
			//	}
			//	break;
		case CHECK_EVASION: case QUIET_CHECKS: case FORKS: case FORKS_NO_CHECKS:
#pragma warning(suppress: 6385)
			move = moves.at(phaseStartIndex + moveIterationPointer).move;
			if (move) {
				++moveIterationPointer;
				goto end_post_hash;
			}
			else {
				++generationPhase;
				moveIterationPointer = -1;
			}
			break;
		case QUIETS_POSITIVE:
			move = moves.at(phaseStartIndex + moveIterationPointer).move;
			if (move == firstNegative->move) {
				++generationPhase;
			}
			else {
				++moveIterationPointer;
				goto end_post_killer;
			}
			break;
		case REPEAT_ALL: case ALL:
#pragma warning(suppress: 6385)
			move = moves.at(moveIterationPointer).move;
			++moveIterationPointer;
			generationPhase += (moveIterationPointer >= movepointer);
			goto end;
		case UNDERPROMOTION:
#pragma warning(suppress: 6385)
			move = moves.at(phaseStartIndex + moveIterationPointer).move;
			++moveIterationPointer;
			generationPhase += (phaseStartIndex + moveIterationPointer >= movepointer);
			goto end_post_hash;
		default:
			assert(true);
		}
	} while (generationPhases.at(generationPhase) != NONE);
	return MOVE_NONE;
end_post_killer:
	if (killerManager != nullptr && (processedMoveGenerationPhases & (1 << static_cast<int>(MoveGenerationType::KILLER))) != 0) {
		if (killerManager->isKiller(*this, move)) return NextMove();
	}
end_post_hash:
	if (hashMove && move == hashMove) return NextMove();
end:
	return move;
}

//Insertion Sort (copied from SF)
void Position::insertionSort(ValuatedMove* first, ValuatedMove* last) noexcept
{
	ValuatedMove tmp, * p, * q = nullptr;
	for (p = first + 1; p < last; ++p)
	{
		tmp = *p;
		for (q = p; q != first && *(q - 1) < tmp; --q) *q = *(q - 1);
		*q = tmp;
	}
}

void Position::shellSort(ValuatedMove* vm, int count) noexcept {
	const int gaps[] = { 57, 23, 10, 4, 1 };
	for (const int gap : gaps) {
		for (int i = 0; i < count - gap; i++) {
			int j = i + gap;
			const ValuatedMove tmp = vm[j];
			while (j >= gap && tmp.score > vm[j - gap].score) {
				vm[j] = vm[j - gap];
				j -= gap;
			}
			vm[j] = tmp;
		}
	}
}

void Position::evaluateByCaptureScore(int startIndex) noexcept {
	for (int i = startIndex; i < movepointer - 1; ++i) {
		moves.at(i).score = settings::parameter.CAPTURE_SCORES.at(GetPieceType(Board.at(from(moves.at(i).move)))).at(GetPieceType(Board.at(to(moves.at(i).move)))) + 2 * (type(moves.at(i).move) == MoveType::PROMOTION);
	}
}

void Position::evaluateBySEE(int startIndex) noexcept {
	if (movepointer - 2 == startIndex)
		moves.at(startIndex).score = settings::parameter.SEE_VAL.at(QUEEN); //No need for SEE if there is only one move to be evaluated
	else
		for (int i = startIndex; i < movepointer - 1; ++i) moves.at(i).score = SEE(moves.at(i).move);
}

void Position::evaluateCheckEvasions(int startIndex) {
	const ValuatedMove* firstQuiet = std::partition(moves.data() + startIndex, &moves.data()[movepointer - 1], [this](ValuatedMove m) noexcept { return IsTactical(m); });
	bool quiets = false;
	int quietsIndex = startIndex;
	for (int i = startIndex; i < movepointer - 1; ++i) {
		quiets = quiets || (moves.at(i).move == firstQuiet->move);
		if (quiets && history) {
			const Piece p = Board.at(from(moves.at(i).move));
			moves.at(i).score = history->getValue(p, moves.at(i).move);
		}
		else {
			moves.at(i).score = settings::parameter.CAPTURE_SCORES.at(GetPieceType(Board.at(from(moves.at(i).move)))).at(GetPieceType(Board.at(to(moves.at(i).move)))) + 2 * (type(moves.at(i).move) == MoveType::PROMOTION);
			quietsIndex++;
		}
	}
	if (quietsIndex > startIndex + 1) insertionSort(moves.data() + startIndex, moves.data() + quietsIndex - 1);
	if (movepointer - 2 > quietsIndex) insertionSort(moves.data() + quietsIndex, &moves.data()[movepointer - 1]);
}

Move Position::GetCounterMove(std::array<std::array<Move, 64>, 12>& counterMoves) noexcept {
	if (lastAppliedMove != MOVE_NONE) {
		const Square lastTo = to(FixCastlingMove(lastAppliedMove));
		return counterMoves.at(int(Board.at(lastTo))).at(lastTo);
	}
	return MOVE_NONE;
}

void Position::evaluateByHistory(int startIndex) noexcept {
	Move lastMoves[2] = { MOVE_NONE, MOVE_NONE };
	Piece lastMovingPieces[2] = { Piece::BLANK, Piece::BLANK };
	if (lastAppliedMove != MOVE_NONE) {
		lastMoves[0] = FixCastlingMove(lastAppliedMove);
		lastMovingPieces[0] = Board.at(to(lastMoves[0]));
		if (Previous() && Previous()->lastAppliedMove != MOVE_NONE) {
			lastMoves[1] = FixCastlingMove(Previous()->lastAppliedMove);
			lastMovingPieces[1] = Previous()->Board.at(to(lastMoves[1]));
		}
	}
	const Bitboard bbNewlyAttacked = lastAppliedMove == MOVE_NONE ? EMPTY : (~(Previous()->attackedByUs)) & AttackedByThem();
	for (int i = startIndex; i < movepointer - 1; ++i) {
		if (moves.at(i).move == counterMove) {
			moves.at(i).score = VALUE_MATE;
		}
		else
			if (history) {
				const Move fixedMove = FixCastlingMove(moves.at(i).move);
				const Square toSquare = to(fixedMove);
				const Piece p = Board.at(from(fixedMove));
				moves.at(i).score = gsl::narrow_cast<Value>(history->getValue(p, fixedMove) - ChebishevDistance(toSquare, KingSquare(~SideToMove))); //History + king tropism if equal
				if (lastMoves[0] && cmHistory) {
					moves.at(i).score += 2 * cmHistory->getValue(lastMovingPieces[0], to(lastMoves[0]), p, toSquare);
					if (lastMoves[1]) moves.at(i).score += 2 * followupHistory->getValue(lastMovingPieces[1], to(lastMoves[1]), p, toSquare);
				}
				const Bitboard toBB = ToBitboard(toSquare);
				if (ToBitboard(from(moves.at(i).move)) & bbNewlyAttacked) moves.at(i).score = gsl::narrow_cast<Value>(moves.at(i).score + 100);
				if ((toBB & (attackedByUs | ~attackedByThem)) != EMPTY)
					moves.at(i).score = gsl::narrow_cast<Value>(moves.at(i).score + 500);
				else if ((p < WPAWN) && (toBB & AttacksByPieceType(~SideToMove, PAWN)) != 0) {
					moves.at(i).score = gsl::narrow_cast<Value>(moves.at(i).score - 500);
				}
				assert(moves.at(i).score < VALUE_MATE);
			}
			else moves.at(i).score = VALUE_DRAW;
	}
}

Move Position::getBestMove(int startIndex) noexcept {
	ValuatedMove bestmove = moves.at(startIndex);
	for (int i = startIndex + 1; i < movepointer - 1; ++i) {
		if (bestmove < moves.at(i)) {
			moves.at(startIndex) = moves.at(i);
			moves.at(i) = bestmove;
			bestmove = moves.at(startIndex);
		}
	}
	return moves.at(startIndex).score > -VALUE_MATE ? moves.at(startIndex).move : MOVE_NONE;
}

template<bool SquareIsEmpty> void Position::set(const Piece piece, const Square square) noexcept {
	const Bitboard squareBB = 1ull << square;
	Piece captured = Piece::BLANK;
	if (!SquareIsEmpty && (captured = Board.at(square)) != BLANK) {
		OccupiedByPieceType.at(GetPieceType(captured)) &= ~squareBB;
		OccupiedByColor.at(static_cast<int>(GetColor(captured))) &= ~squareBB;
		Hash ^= ZobristKeys.at(captured).at(square);
		PsqEval -= settings::parameter.PSQT.at(captured).at(square);
	}
	OccupiedByPieceType.at(GetPieceType(piece)) |= squareBB;
	OccupiedByColor.at(static_cast<int>(GetColor(piece))) |= squareBB;
	Board.at(square) = piece;
	PsqEval += settings::parameter.PSQT.at(piece).at(square);
	Hash ^= ZobristKeys.at(piece).at(square);
}

void Position::remove(const Square square) noexcept {
	const Bitboard NotSquareBB = ~(1ull << square);
	const Piece piece = Board.at(square);
	Board.at(square) = BLANK;
	PsqEval -= settings::parameter.PSQT.at(piece).at(square);
	OccupiedByPieceType.at(GetPieceType(piece)) &= NotSquareBB;
	OccupiedByColor.at(static_cast<int>(GetColor(piece))) &= NotSquareBB;
	Hash ^= ZobristKeys.at(piece).at(square);
}

void Position::updateCastleFlags(Square fromSquare, Square toSquare) noexcept {
	assert(GetPieceType(Board.at(fromSquare)) != KING || std::abs(static_cast<int>(fromSquare) - static_cast<int>(toSquare)) != 2 || Chess960);
	if (GetCastlesForColor(SideToMove) != CastleFlag::NoCastles) {
		if (fromSquare == InitialKingSquare.at(static_cast<int>(SideToMove))) {
			RemoveCastlingOption(static_cast<CastleFlag>(W0_0 << (2 * static_cast<int>(SideToMove))));
			RemoveCastlingOption(static_cast<CastleFlag>(W0_0_0 << (2 * static_cast<int>(SideToMove))));
			SetCastlingLost(SideToMove);
		}
		else {
			const Color other = ~SideToMove;
			if (fromSquare == InitialRookSquare.at(2 * static_cast<int>(SideToMove))) {
				RemoveCastlingOption(static_cast<CastleFlag>(W0_0 << (2 * static_cast<int>(SideToMove))));
				if (GetCastlesForColor(SideToMove) == CastleFlag::NoCastles) SetCastlingLost(SideToMove);
			}
			else if (fromSquare == InitialRookSquare.at(2 * static_cast<int>(SideToMove) + 1)) {
				RemoveCastlingOption(static_cast<CastleFlag>(W0_0_0 << (2 * static_cast<int>(SideToMove))));
				if (GetCastlesForColor(SideToMove) == CastleFlag::NoCastles) SetCastlingLost(SideToMove);
			}
			if (toSquare == InitialRookSquare.at(2 * static_cast<int>(other))) {
				RemoveCastlingOption(static_cast<CastleFlag>(W0_0 << (2 * static_cast<int>(other))));
				if (GetCastlesForColor(other) == CastleFlag::NoCastles) SetCastlingLost(other);
			}
			else if (toSquare == InitialRookSquare.at(2 * static_cast<int>(other) + 1)) {
				RemoveCastlingOption(static_cast<CastleFlag>(W0_0_0 << (2 * static_cast<int>(other))));
				if (GetCastlesForColor(other) == CastleFlag::NoCastles) SetCastlingLost(other);
			}
		}
	}
}

Bitboard Position::calculateAttacks(Color color) noexcept {
	const Bitboard occupied = OccupiedBB();
	attacksByPt.at(GetPiece(ROOK, color)) = 0ull;
	Bitboard rookSliders = PieceBB(ROOK, color);
	Bitboard bbAttacks = EMPTY;
	dblAttacked.at(static_cast<int>(color)) = EMPTY;
	while (rookSliders) {
		const Square sq = lsb(rookSliders);
		const Bitboard a = RookTargets(sq, occupied);
		dblAttacked.at(static_cast<int>(color)) |= bbAttacks & a;
		attacks.at(sq) = a;
		bbAttacks |= a;
		attacksByPt.at(GetPiece(ROOK, color)) |= attacks.at(sq);
		rookSliders &= rookSliders - 1;
	}
	attacksByPt.at(GetPiece(BISHOP, color)) = 0ull;
	Bitboard bishopSliders = PieceBB(BISHOP, color);
	while (bishopSliders) {
		const Square sq = lsb(bishopSliders);
		const Bitboard a = BishopTargets(sq, occupied);
		dblAttacked.at(static_cast<int>(color)) |= bbAttacks & a;
		attacks.at(sq) = a;
		bbAttacks |= a;
		attacksByPt.at(GetPiece(BISHOP, color)) |= attacks.at(sq);
		bishopSliders &= bishopSliders - 1;
	}
	attacksByPt.at(GetPiece(QUEEN, color)) = 0ull;
	Bitboard queenSliders = PieceBB(QUEEN, color);
	while (queenSliders) {
		const Square sq = lsb(queenSliders);
		Bitboard a = RookTargets(sq, occupied);
		a |= BishopTargets(sq, occupied);
		dblAttacked.at(static_cast<int>(color)) |= bbAttacks & a;
		attacks.at(sq) = a;
		bbAttacks |= a;
		attacksByPt.at(GetPiece(QUEEN, color)) |= attacks.at(sq);
		queenSliders &= queenSliders - 1;
	}
	attacksByPt.at(GetPiece(KNIGHT, color)) = 0ull;
	Bitboard knights = PieceBB(KNIGHT, color);
	while (knights) {
		const Square sq = lsb(knights);
		const Bitboard a = KnightAttacks.at(sq);
		dblAttacked.at(static_cast<int>(color)) |= bbAttacks & a;
		attacks.at(sq) = a;
		bbAttacks |= a;
		attacksByPt.at(GetPiece(KNIGHT, color)) |= attacks.at(sq);
		knights &= knights - 1;
	}
	const Square kingSquare = kingSquares.at(static_cast<int>(color));
	attacks.at(kingSquare) = KingAttacks.at(kingSquare);
	dblAttacked.at(static_cast<int>(color)) |= bbAttacks & attacks.at(kingSquare);
	bbAttacks |= attacks.at(kingSquare);
	attacksByPt.at(GetPiece(KING, color)) = attacks.at(kingSquare);
	attacksByPt.at(GetPiece(PAWN, color)) = 0ull;
	Bitboard pawns = PieceBB(PAWN, color);
	while (pawns) {
		const Square sq = lsb(pawns);
		const Bitboard a = PawnAttacks.at(static_cast<int>(color)).at(sq);
		dblAttacked.at(static_cast<int>(color)) |= bbAttacks & a;
		attacks.at(sq) = a;
		bbAttacks |= a;
		attacksByPt.at(GetPiece(PAWN, color)) |= attacks.at(sq);
		pawns &= pawns - 1;
	}
	//dblAttacked.at(static_cast<int>(color)) |= BatteryAttacks(color);
	return bbAttacks;
}

//Battery attacks are squares attacked by rooks or queens backed by a Slider behind
Bitboard Position::BatteryAttacks(Color attacking_color) const noexcept {
	Bitboard bbXRay = EMPTY;
	Bitboard bbRooks = OccupiedByColor.at(static_cast<int>(attacking_color)) & (OccupiedByPieceType.at(QUEEN) | OccupiedByPieceType.at(ROOK));
	const Bitboard bbExclRooks = OccupiedBB() & ~bbRooks;
	while (bbRooks) {
		const Square s = lsb(bbRooks);
		bbXRay |= RookTargets(s, bbExclRooks) & ~attacks.at(s);
		bbRooks &= bbRooks - 1;
	}
	Bitboard bbBishops = OccupiedByColor.at(static_cast<int>(attacking_color)) & (OccupiedByPieceType.at(QUEEN) | OccupiedByPieceType.at(BISHOP));
	const Bitboard bbExclQueen = OccupiedBB() & ~PieceBB(QUEEN, attacking_color);
	while (bbBishops) {
		const Square s = lsb(bbBishops);
		bbXRay |= BishopTargets(s, bbExclQueen) & ~attacks.at(s);
		bbBishops &= bbBishops - 1;
	}
	return bbXRay;
}

void Position::pack(uint8_t* data, int move_count) const
{
	utils::BitStream stream;
	memset(data, 0, 32 /* 256bit */);
	stream.set_data(data);

	// turn
	// Side to move.
	stream.write_one_bit(static_cast<int>(SideToMove));

	// 7-bit positions for leading and trailing balls
	// White king and black king, 6 bits for each.
	for (int c = 0; c < 2; ++c)
		stream.write_n_bit(kingSquares.at(c), 6);

	// Write the pieces on the board other than the kings.
	for (int r = 7; r >= 0; --r)
	{
		for (int f = 0; f <= 7; ++f)
		{
			const Piece pc = Board.at(8 * r + f);
			const PieceType pt = GetPieceType(pc);
			if (pt == PieceType::KING)
				continue;
			auto c = utils::huffman_table.at(pt);
			stream.write_n_bit(c.code, c.bits);

			if (pt == PieceType::NO_TYPE)
				continue;
			// first and second flag
			stream.write_one_bit(pc & 1);
		}
	}

	// TODO(someone): Support chess960.
	stream.write_one_bit(CastlingOptions & CastleFlag::W0_0);
	stream.write_one_bit(CastlingOptions & CastleFlag::W0_0_0);
	stream.write_one_bit(CastlingOptions & CastleFlag::B0_0);
	stream.write_one_bit(CastlingOptions & CastleFlag::B0_0_0);

	if (EPSquare == Square::OUTSIDE) {
		stream.write_one_bit(0);
	}
	else {
		stream.write_one_bit(1);
		stream.write_n_bit(static_cast<int>(EPSquare), 6);
	}

	stream.write_n_bit(DrawPlyCount, 6);

	stream.write_n_bit(move_count, 8);

	assert(stream.get_cursor() <= 256);
}

void Position::unpack(uint8_t* data)
{
	CastlingOptions = CastleFlag::NoCastles;
	material = nullptr;
	Board.fill(Piece::BLANK);
	OccupiedByColor.at(static_cast<int>(Color::WHITE)) = OccupiedByColor.at(static_cast<int>(Color::BLACK)) = 0ull;
	OccupiedByPieceType.fill(Piece::BLANK);
	EPSquare = OUTSIDE;
	SideToMove = Color::WHITE;
	DrawPlyCount = 0;
	AppliedMovesBeforeRoot = 0;
	Hash = ZobristMoveColor;
	PsqEval = EVAL_ZERO;

	utils::BitStream stream;
	stream.set_data(data);
	if (GetSideToMove() != static_cast<Color>(stream.read_one_bit())) SwitchSideToMove();

	kingSquares.at(static_cast<int>(Color::WHITE)) = static_cast<Square>(stream.read_n_bit(6));
	kingSquares.at(static_cast<int>(Color::BLACK)) = static_cast<Square>(stream.read_n_bit(6));
	set<true>(Piece::WKING, kingSquares.at(static_cast<int>(Color::WHITE)));
	set<true>(Piece::BKING, kingSquares.at(static_cast<int>(Color::BLACK)));

	for (int r = 7; r >= 0; --r)
	{
		for (int f = 0; f <= 7; ++f)
		{
			const Square s = static_cast<Square>(8 * r + f);
			if (s == kingSquares.at(static_cast<int>(Color::WHITE)) || s == kingSquares.at(static_cast<int>(Color::BLACK))) continue;
			PieceType pr = PieceType::NO_TYPE;
			int code = 0, bits = 0;
			while (true)
			{
				code |= stream.read_one_bit() << bits;
				++bits;

				assert(bits <= 6);
				if (utils::huffman_table.at(PieceType::NO_TYPE).code == code && utils::huffman_table.at(pr).bits == bits)
					break;
				if (utils::huffman_table.at(PAWN).code == code
					&& utils::huffman_table.at(PAWN).bits == bits) {
					pr = PAWN;
					break;
				}
				if (utils::huffman_table.at(ROOK).code == code
					&& utils::huffman_table.at(ROOK).bits == bits) {
					pr = ROOK;
					break;
				}
				if (utils::huffman_table.at(BISHOP).code == code
					&& utils::huffman_table.at(BISHOP).bits == bits) {
					pr = BISHOP;
					break;
				}
				if (utils::huffman_table.at(KNIGHT).code == code
					&& utils::huffman_table.at(KNIGHT).bits == bits) {
					pr = KNIGHT;
					break;
				}
				if (utils::huffman_table.at(QUEEN).code == code
					&& utils::huffman_table.at(QUEEN).bits == bits) {
					pr = QUEEN;
					break;
				}
			}
			if (pr == PieceType::NO_TYPE)
				continue;

			// first and second flag
			const Color c = static_cast<Color>(stream.read_one_bit());
			const Piece p = GetPiece(pr, c);
			set<true>(p, s);
		}
	}

	if (stream.read_one_bit() == 1) {
		AddCastlingOption(W0_0);
		InitialKingSquareBB.at(static_cast<int>(Color::WHITE)) = PieceBB(KING, Color::WHITE);
		InitialKingSquare.at(static_cast<int>(Color::WHITE)) = lsb(InitialKingSquareBB.at(static_cast<int>(Color::WHITE)));
		for (Square k = InitialKingSquare.at(static_cast<int>(Color::WHITE)); k <= H1; ++k) {
			if (Board.at(k) == WROOK) {
				InitialRookSquare.at(0) = k;
				break;
			}
		}
	}
	if (stream.read_one_bit() == 1) {
		AddCastlingOption(W0_0_0);
		InitialKingSquareBB.at(static_cast<int>(Color::WHITE)) = PieceBB(KING, Color::WHITE);
		InitialKingSquare.at(static_cast<int>(Color::WHITE)) = lsb(InitialKingSquareBB.at(static_cast<int>(Color::WHITE)));
#pragma warning(suppress: 6295)
		for (Square k = InitialKingSquare.at(static_cast<int>(Color::WHITE)); k >= A1; --k) {
			if (Board.at(k) == WROOK) {
				InitialRookSquare.at(1) = k;
				break;
			}
		}
	}
	if (stream.read_one_bit() == 1) {
		AddCastlingOption(B0_0);
		InitialKingSquareBB.at(static_cast<int>(Color::BLACK)) = PieceBB(KING, Color::BLACK);
		InitialKingSquare.at(static_cast<int>(Color::BLACK)) = lsb(InitialKingSquareBB.at(static_cast<int>(Color::BLACK)));
		for (Square k = InitialKingSquare.at(static_cast<int>(Color::BLACK)); k <= H8; ++k) {
			if (Board.at(k) == BROOK) {
				InitialRookSquare.at(2) = k;
				break;
			}
		}
	}
	if (stream.read_one_bit() == 1) {
		AddCastlingOption(B0_0_0);
		InitialKingSquareBB.at(static_cast<int>(Color::BLACK)) = PieceBB(KING, Color::BLACK);
		InitialKingSquare.at(static_cast<int>(Color::BLACK)) = lsb(InitialKingSquareBB.at(static_cast<int>(Color::BLACK)));
		for (Square k = InitialKingSquare.at(static_cast<int>(Color::BLACK)); k >= A8; --k) {
			if (Board.at(k) == BROOK) {
				InitialRookSquare.at(3) = k;
				break;
			}
		}
	}

	if (CastlingOptions & 15) {
		Chess960 = Chess960 || (InitialKingSquare.at(static_cast<int>(Color::WHITE)) != E1) || (InitialRookSquare.at(0) != H1) || (InitialRookSquare.at(1) != A1);
		for (int i = 0; i < 4; ++i) InitialRookSquareBB.at(i) = 1ull << InitialRookSquare.at(i);
		const std::array<Square,4> kt = { G1, C1, G8, C8 };
		const std::array<Square, 4> rt = { F1, D1, F8, D8 };
		for (int i = 0; i < 4; ++i) {
			SquaresToBeEmpty.at(i) = 0ull;
			SquaresToBeUnattacked.at(i) = 0ull;
			const Square ks = lsb(InitialKingSquareBB.at(i / 2));
			for (int j = std::min(ks, kt.at(i)); j <= std::max(ks, kt.at(i)); ++j) SquaresToBeUnattacked.at(i) |= 1ull << j;
			for (int j = std::min(InitialRookSquare.at(i), rt.at(i)); j <= std::max(InitialRookSquare.at(i), rt.at(i)); ++j) {
				SquaresToBeEmpty.at(i) |= 1ull << j;
			}
			for (int j = std::min(ks, kt.at(i)); j <= std::max(ks, kt.at(i)); ++j) {
				SquaresToBeEmpty.at(i) |= 1ull << j;
			}
			SquaresToBeEmpty.at(i) &= ~InitialKingSquareBB.at(i / 2);
			SquaresToBeEmpty.at(i) &= ~InitialRookSquareBB.at(i);
		}
	}

	if (stream.read_one_bit()) {
		EPSquare = static_cast<Square>(stream.read_n_bit(6));
		Hash ^= ZobristEnPassant.at(EPSquare & 7);
	}
	else {
		EPSquare = OUTSIDE;
	}

	DrawPlyCount = static_cast<Square>(stream.read_n_bit(6));
	attacks.fill(EMPTY);
	PawnKey = calculatePawnKey();
	pawn = pawn::probe(*this);
	if (checkMaterialIsUnusual()) {
		MaterialKey = MATERIAL_KEY_UNUSUAL;
		material = initUnusual(*this);
	}
	else {
		MaterialKey = calculateMaterialKey();
		material = probe(MaterialKey);
	}
	attackedByThem = calculateAttacks(~SideToMove);
	attackedByUs = calculateAttacks(SideToMove);
	CalculatePinnedPieces();
	pliesFromRoot = 0;
}

int16_t Position::NNScore() const
{
	nnInput.kingSquares.at(0) = kingSquares.at(0);
	nnInput.kingSquares.at(1) = kingSquares.at(1);
	nnInput.is_black_to_move = SideToMove == Color::BLACK;
	Bitboard occ = OccupiedBB() & ~PieceTypeBB(PieceType::KING);
	std::array<nnue::Piece, 64> nboard;
	nboard.fill(nnue::Piece::NONE);
	nnInput.board = nboard.data();
	while (occ) {
		Square s = lsb(occ);
		nnInput.board[static_cast<int>(s)] = TransformPiece(Board.at(s));
		occ &= occ - 1;
	}
	nnInput.board[nnInput.kingSquares.at(0)] = nnue::Piece::WKING;
	nnInput.board[nnInput.kingSquares.at(1)] = nnue::Piece::BKING;
	nnInput.set = previous != nullptr && previous->nnInput.set;
	int16_t v = network.score(nnInput);
	v = gsl::narrow_cast<int16_t>(settings::parameter.NNScaleFactor * v / 128);
	return std::clamp(v, static_cast<int16_t>(-VALUE_MATE + 2 * MAX_DEPTH + 1), static_cast<int16_t>(VALUE_MATE - 2 * MAX_DEPTH - 1));
}

Bitboard Position::checkBlocker(Color colorOfBlocker, Color kingColor) noexcept {
	return (PinnedPieces(kingColor) & OccupiedByColor.at(static_cast<int>(colorOfBlocker)));
}

//Calculates a bitboard of attackers to a given square, but filtering pieces by occupancy mask 
Bitboard Position::AttacksOfField(const Square targetField, const Bitboard occupanyMask) const noexcept
{
	//sliding attacks
	Bitboard attacksOfField = RookTargets(targetField, occupanyMask) & (OccupiedByPieceType.at(ROOK) | OccupiedByPieceType.at(QUEEN));
	attacksOfField |= BishopTargets(targetField, occupanyMask) & (OccupiedByPieceType.at(BISHOP) | OccupiedByPieceType.at(QUEEN));
	attacksOfField &= occupanyMask;
	//non-sliding attacks
	attacksOfField |= KnightAttacks.at(targetField) & OccupiedByPieceType.at(KNIGHT);
	attacksOfField |= KingAttacks.at(targetField) & OccupiedByPieceType.at(KING);
	const Bitboard targetBB = ToBitboard(targetField);
	attacksOfField |= ((targetBB >> 7) & NOT_A_FILE) & PieceBB(PAWN, Color::WHITE);
	attacksOfField |= ((targetBB >> 9) & NOT_H_FILE) & PieceBB(PAWN, Color::WHITE);
	attacksOfField |= ((targetBB << 7) & NOT_H_FILE) & PieceBB(PAWN, Color::BLACK);
	attacksOfField |= ((targetBB << 9) & NOT_A_FILE) & PieceBB(PAWN, Color::BLACK);
	return attacksOfField & occupanyMask;
}

Bitboard Position::AttacksOfField(const Square targetField, const Color attackingSide) const noexcept {
	//sliding attacks
	Bitboard attacksOfField = SlidingAttacksRookTo.at(targetField) & (OccupiedByPieceType.at(ROOK) | OccupiedByPieceType.at(QUEEN));
	attacksOfField |= SlidingAttacksBishopTo.at(targetField) & (OccupiedByPieceType.at(BISHOP) | OccupiedByPieceType.at(QUEEN));
	attacksOfField &= OccupiedByColor.at(static_cast<int>(attackingSide));
	//Check for blockers
	Bitboard tmpAttacks = attacksOfField;
	while (tmpAttacks != 0)
	{
		const Square from = lsb(tmpAttacks);
		const Bitboard blocker = InBetweenFields.at(from).at(targetField) & OccupiedBB();
		if (blocker) attacksOfField &= ~ToBitboard(from);
		tmpAttacks &= tmpAttacks - 1;
	}
	//non-sliding attacks
	attacksOfField |= KnightAttacks.at(targetField) & OccupiedByPieceType.at(KNIGHT);
	attacksOfField |= KingAttacks.at(targetField) & OccupiedByPieceType.at(KING);
	const Bitboard targetBB = ToBitboard(targetField);
	attacksOfField |= ((targetBB >> 7) & NOT_A_FILE) & PieceBB(PAWN, Color::WHITE);
	attacksOfField |= ((targetBB >> 9) & NOT_H_FILE) & PieceBB(PAWN, Color::WHITE);
	attacksOfField |= ((targetBB << 7) & NOT_H_FILE) & PieceBB(PAWN, Color::BLACK);
	attacksOfField |= ((targetBB << 9) & NOT_A_FILE) & PieceBB(PAWN, Color::BLACK);
	attacksOfField &= OccupiedByColor.at(static_cast<int>(attackingSide));
	return attacksOfField;
}


PieceType Position::getAndResetLeastValuableAttacker(Square toSquare, Bitboard attackers, Bitboard& occupied, Bitboard& attadef, Bitboard& mayXray) const noexcept {
	Bitboard leastAttackers = attackers & PieceTypeBB(PAWN);
	if (!leastAttackers) leastAttackers = attackers & (PieceTypeBB(KNIGHT) | PieceTypeBB(BISHOP));
	if (!leastAttackers) leastAttackers = attackers & PieceTypeBB(ROOK);
	if (!leastAttackers) leastAttackers = attackers & PieceTypeBB(QUEEN);
	if (!leastAttackers) leastAttackers = attackers & PieceTypeBB(KING);
	assert(leastAttackers);
	leastAttackers &= (0 - leastAttackers);
	const Square leastAttackerSquare = lsb(leastAttackers);
	occupied &= ~leastAttackers;
	attadef &= ~leastAttackers;
	//Check if there is an xray attack now 
	const Bitboard shadowed = ShadowedFields.at(toSquare).at(leastAttackerSquare);
	Bitboard xray = mayXray & shadowed;
	while (xray) {
		const Square xraySquare = lsb(xray);
		if ((InBetweenFields.at(xraySquare).at(toSquare) & occupied) == 0) {
			attadef |= ToBitboard(xraySquare);
			mayXray &= ~attadef;
			break;
		}
		xray &= xray - 1;
	}
	return GetPieceType(Board.at(leastAttackerSquare));
}

//SEE method, which returns without exact value, when it's sure that value is positive (then VALUE_KNOWN_WIN is returned)
Value Position::SEE_Sign(Move move) const noexcept {
	const Square fromSquare = from(move);
	const Square toSquare = to(move);
	if (settings::parameter.SEE_VAL.at(GetPieceType(Board.at(fromSquare))) <= settings::parameter.SEE_VAL.at(GetPieceType(Board.at(toSquare))) && type(move) != MoveType::PROMOTION) return VALUE_KNOWN_WIN;
	return SEE(move);
}

//This SEE implementation is a mixture of stockfish and Computer chess sample implementation
//(see https://chessprogramming.wikispaces.com/SEE+-+The+Swap+Algorithm)
Value Position::SEE(Move move) const noexcept
{
	if (type(move) == MoveType::CASTLING) return VALUE_ZERO;

	std::array<Value, 32> gain{ 0 };
	int d = 1;

	const Square fromSquare = from(move);
	const Square toSquare = to(move);
	gain.at(0) = settings::parameter.SEE_VAL.at(GetPieceType(Board.at(toSquare)));
	Color side = GetColor(Board.at(fromSquare));
	const Bitboard fromBB = ToBitboard(fromSquare);
	Bitboard occ = OccupiedBB() & (~fromBB) & (~ToBitboard(toSquare));

	if (type(move) == MoveType::ENPASSANT)
	{
		occ ^= toSquare - 8 * (1 - 2 * static_cast<int>(side));
		gain.at(0) = settings::parameter.SEE_VAL.at(PAWN);
	}

	// Get Attackers ofto Square
	Bitboard attadef = AttacksOfField(toSquare, occ);
	// Get potential xray attackers
	Bitboard mayXRay = SlidingAttacksRookTo.at(toSquare) & (OccupiedByPieceType.at(ROOK) | OccupiedByPieceType.at(QUEEN));
	mayXRay |= SlidingAttacksBishopTo.at(toSquare) & (OccupiedByPieceType.at(BISHOP) | OccupiedByPieceType.at(QUEEN));
	mayXRay &= occ;
	mayXRay &= ~attadef;

	// If there are no attackers we are done (we had a simple capture of a hanging piece)
	side = ~side;
	Bitboard attackers = attadef & OccupiedByColor.at(static_cast<int>(side));

	//Consider pinned pieces
	if ((attackers & PinnedPieces(side)) != EMPTY && (bbPinner.at(static_cast<int>(side)) & occ) != EMPTY)
		attackers &= ~PinnedPieces(side);

	if (!attackers) return gain.at(0);

	PieceType capturingPiece = GetPieceType(Board.at(fromSquare));

	do {
		assert(d < 32);

		// Add the new entry to the swap list
		gain.at(d) = -gain.at(d - 1) + settings::parameter.SEE_VAL.at(capturingPiece);

		// Locate and remove the next least valuable attacker
		capturingPiece = getAndResetLeastValuableAttacker(toSquare, attackers, occ, attadef, mayXRay);
		side = ~side;
		attackers = attadef & OccupiedByColor.at(static_cast<int>(side));
		//Consider pinned pieces
		if ((attackers & PinnedPieces(side)) != EMPTY && (bbPinner.at(static_cast<int>(side)) & occ) != EMPTY)
			attackers &= ~PinnedPieces(side);
		++d;

	} while (attackers && (capturingPiece != KING || (--d, false))); // Stop before a king capture

	if (capturingPiece == PAWN && (toSquare <= H1 || toSquare >= A8)) {
		gain.at(d - 1) += settings::parameter.SEE_VAL.at(QUEEN) - settings::parameter.SEE_VAL.at(PAWN);
	}

	// find the best achievable score by minimaxing
	while (--d) gain.at(d - 1) = std::min(static_cast<Value>(-gain.at(d)), gain.at(d - 1));

	return gain.at(0);
}


void Position::setFromFEN(const std::string& fen) {
	material = nullptr;
	Board.fill(Piece::BLANK);
	OccupiedByColor.at(static_cast<int>(Color::WHITE)) = OccupiedByColor.at(static_cast<int>(Color::BLACK)) = 0ull;
	OccupiedByPieceType.fill(EMPTY);
	EPSquare = OUTSIDE;
	SideToMove = Color::WHITE;
	DrawPlyCount = 0;
	AppliedMovesBeforeRoot = 0;
	Hash = ZobristMoveColor;
	PsqEval = EVAL_ZERO;
	std::istringstream ss(fen);
	ss >> std::noskipws;
	unsigned char token = ' ';

	//Piece placement
	size_t piece = 0;
	char square = A8;
	while ((ss >> token) && !isspace(token)) {
		if (isdigit(token))
			square = square + (token - '0');
		else if (token == '/')
			square -= 16;
		else if ((piece = PieceToChar.find(token)) != std::string::npos) {
			set<true>(static_cast<Piece>(piece), static_cast<Square>(square));
			square++;
		}
	}

	kingSquares.at(static_cast<int>(Color::WHITE)) = lsb(PieceBB(KING, Color::WHITE));
	kingSquares.at(static_cast<int>(Color::BLACK)) = lsb(PieceBB(KING, Color::BLACK));

	//Side to Move
	ss >> token;
	if (token != 'w') SwitchSideToMove();

	//Castles
	CastlingOptions = 0;
	ss >> token;
	while ((ss >> token) && !isspace(token)) {

		if (token == 'K') {
			AddCastlingOption(CastleFlag::W0_0);
			InitialKingSquareBB.at(static_cast<int>(Color::WHITE)) = PieceBB(KING, Color::WHITE);
			InitialKingSquare.at(static_cast<int>(Color::WHITE)) = lsb(InitialKingSquareBB.at(static_cast<int>(Color::WHITE)));
			for (Square k = InitialKingSquare.at(static_cast<int>(Color::WHITE)); k <= H1; ++k) {
				if (Board.at(k) == WROOK) {
					InitialRookSquare.at(0) = k;
					break;
				}
			}
		}
		else if (token == 'Q') {
			AddCastlingOption(CastleFlag::W0_0_0);
			InitialKingSquareBB.at(static_cast<int>(Color::WHITE)) = PieceBB(KING, Color::WHITE);
			InitialKingSquare.at(static_cast<int>(Color::WHITE)) = lsb(InitialKingSquareBB.at(static_cast<int>(Color::WHITE)));
#pragma warning(suppress: 6295)
			for (Square k = InitialKingSquare.at(static_cast<int>(Color::WHITE)); k >= A1 && k <= H8; --k) {
				if (Board.at(k) == WROOK) {
					InitialRookSquare.at(1) = k;
					break;
				}
			}
		}
		else if (token == 'k') {
			AddCastlingOption(CastleFlag::B0_0);
			InitialKingSquareBB.at(static_cast<int>(Color::BLACK)) = PieceBB(KING, Color::BLACK);
			InitialKingSquare.at(static_cast<int>(Color::BLACK)) = lsb(InitialKingSquareBB.at(static_cast<int>(Color::BLACK)));
			for (Square k = InitialKingSquare.at(static_cast<int>(Color::BLACK)); k <= H8; ++k) {
				if (Board.at(k) == BROOK) {
					InitialRookSquare.at(2) = k;
					break;
				}
			}
		}
		else if (token == 'q') {
			AddCastlingOption(CastleFlag::B0_0_0);
			InitialKingSquareBB.at(static_cast<int>(Color::BLACK)) = PieceBB(KING, Color::BLACK);
			InitialKingSquare.at(static_cast<int>(Color::BLACK)) = lsb(InitialKingSquareBB.at(static_cast<int>(Color::BLACK)));
			for (Square k = InitialKingSquare.at(static_cast<int>(Color::BLACK)); k >= A8; --k) {
				if (Board.at(k) == BROOK) {
					InitialRookSquare.at(3) = k;
					break;
				}
			}

		}
		else if (token == '-') continue;
		else {
			if (token >= 'A' && token <= 'H') {
				const File kingFile = gsl::narrow_cast<File>(kingSquares.at(static_cast<int>(Color::WHITE)) & 7);
				InitialKingSquareBB.at(static_cast<int>(Color::WHITE)) = PieceBB(KING, Color::WHITE);
				InitialKingSquare.at(static_cast<int>(Color::WHITE)) = lsb(InitialKingSquareBB.at(static_cast<int>(Color::WHITE)));
				const File rookFile = gsl::narrow_cast<File>((int)token - (int)'A');
				if (rookFile < kingFile) {
					AddCastlingOption(W0_0_0);
					InitialRookSquare.at(1) = static_cast<Square>(rookFile);
				}
				else {
					AddCastlingOption(W0_0);
					InitialRookSquare.at(0) = static_cast<Square>(rookFile);
				}
			}
			else if (token >= 'a' && token <= 'h') {
				const File kingFile = gsl::narrow_cast<File>(kingSquares.at(static_cast<int>(Color::BLACK)) & 7);
				InitialKingSquareBB.at(static_cast<int>(Color::BLACK)) = PieceBB(KING, Color::BLACK);
				InitialKingSquare.at(static_cast<int>(Color::BLACK)) = lsb(InitialKingSquareBB.at(static_cast<int>(Color::BLACK)));
				const File rookFile = gsl::narrow_cast<File>((int)token - (int)'a');
				if (rookFile < kingFile) {
					AddCastlingOption(B0_0_0);
					InitialRookSquare.at(3) = static_cast<Square>(rookFile + 56);
				}
				else {
					AddCastlingOption(B0_0);
					InitialRookSquare.at(2) = static_cast<Square>(rookFile + 56);
				}
			}
		}
	}
	if (CastlingOptions & 15) {
		Chess960 = Chess960 || (InitialKingSquare.at(static_cast<int>(Color::WHITE)) != E1) || (InitialRookSquare.at(0) != H1) || (InitialRookSquare.at(1) != A1);
		for (int i = 0; i < 4; ++i) InitialRookSquareBB.at(i) = 1ull << InitialRookSquare.at(i);
		const std::array<Square, 4> kt = { G1, C1, G8, C8 };
		const std::array<Square, 4> rt = { F1, D1, F8, D8 };
		for (int i = 0; i < 4; ++i) {
			SquaresToBeEmpty.at(i) = 0ull;
			SquaresToBeUnattacked.at(i) = 0ull;
			const Square ks = lsb(InitialKingSquareBB.at(i / 2));
			for (int j = std::min(ks, kt.at(i)); j <= std::max(ks, kt.at(i)); ++j) SquaresToBeUnattacked.at(i) |= 1ull << j;
			for (int j = std::min(InitialRookSquare.at(i), rt.at(i)); j <= std::max(InitialRookSquare.at(i), rt.at(i)); ++j) {
				SquaresToBeEmpty.at(i) |= 1ull << j;
			}
			for (int j = std::min(ks, kt.at(i)); j <= std::max(ks, kt.at(i)); ++j) {
				SquaresToBeEmpty.at(i) |= 1ull << j;
			}
			SquaresToBeEmpty.at(i) &= ~InitialKingSquareBB.at(i / 2);
			SquaresToBeEmpty.at(i) &= ~InitialRookSquareBB.at(i);
		}
	}

	//EP-square
	char col = 'a', row = '1';
	if (((ss >> col) && (col >= 'a' && col <= 'h'))
		&& ((ss >> row) && (row == '3' || row == '6'))) {
		EPSquare = static_cast<Square>(8 * (row - '0' - 1) + col - 'a');
		Square to = Square::OUTSIDE;
		if (SideToMove == Color::BLACK && EPSquare <= H4) {
			to = static_cast<Square>(EPSquare + 8);
			if ((GetEPAttackersForToField(to) & PieceBB(PAWN, Color::BLACK)) == 0)
				EPSquare = OUTSIDE;
		}
		else if (SideToMove == Color::WHITE && EPSquare >= A5) {
			to = static_cast<Square>(EPSquare - 8);
			if ((GetEPAttackersForToField(to) & PieceBB(PAWN, Color::WHITE)) == 0)
				EPSquare = OUTSIDE;
		}
		if (EPSquare != OUTSIDE) Hash ^= ZobristEnPassant.at(EPSquare & 7);
	}
	std::string dpc;
	ss >> std::skipws >> dpc;
	if (dpc.length() > 0) {
		DrawPlyCount = gsl::narrow_cast<unsigned char>(atoi(dpc.c_str()));
	}
	attacks.fill(EMPTY);
	PawnKey = calculatePawnKey();
	pawn = pawn::probe(*this);
	if (checkMaterialIsUnusual()) {
		MaterialKey = MATERIAL_KEY_UNUSUAL;
		material = initUnusual(*this);
	}
	else {
		MaterialKey = calculateMaterialKey();
		material = probe(MaterialKey);
	}
	attackedByThem = calculateAttacks(~SideToMove);
	attackedByUs = calculateAttacks(SideToMove);
	CalculatePinnedPieces();
	pliesFromRoot = 0;
}

std::string Position::fen(bool only_epd) const {

	int emptyCnt = 0;
	std::ostringstream ss;

	for (Rank r = Rank8; r >= Rank1; --r) {
		for (File f = FileA; f <= FileH; ++f) {
			for (emptyCnt = 0; f <= FileH && Board.at(createSquare(r, f)) == BLANK; ++f)
				++emptyCnt;
			if (emptyCnt)
				ss << emptyCnt;

			if (f <= FileH)
				ss << PieceToChar.at(Board.at(createSquare(r, f)));
		}

		if (r > Rank1)
			ss << '/';
	}

	ss << (SideToMove == Color::WHITE ? " w " : " b ");

	if (!(CastlingOptions & 15))
		ss << '-';
	else if (Chess960) {
		if (W0_0 & CastlingOptions) {
			Bitboard sideOfKing = 0ull;
			for (int i = InitialKingSquare.at(static_cast<int>(Color::WHITE)) + 1; i < 8; ++i) sideOfKing |= 1ull << i;
			sideOfKing &= ~InitialRookSquareBB.at(0);
			if (sideOfKing & PieceBB(ROOK, Color::WHITE)) ss << gsl::narrow_cast<char>(static_cast<int>('A') + InitialRookSquare.at(0)); else ss << 'K';
		}
		if (W0_0_0 & CastlingOptions) {
			Bitboard sideOfKing = 0ull;
			for (int i = InitialKingSquare.at(static_cast<int>(Color::WHITE)) + 1; i >= 0; --i) sideOfKing |= 1ull << i;
			sideOfKing &= ~InitialRookSquareBB.at(1);
			if (sideOfKing & PieceBB(ROOK, Color::WHITE)) ss << gsl::narrow_cast<char>(static_cast<int>('A') + InitialRookSquare.at(1)); else ss << 'Q';
		}
		if (B0_0 & CastlingOptions) {
			Bitboard sideOfKing = 0ull;
			for (int i = InitialKingSquare.at(static_cast<int>(Color::BLACK)) + 1; i < 64; ++i) sideOfKing |= 1ull << i;
			sideOfKing &= ~InitialRookSquareBB.at(2);
			if (sideOfKing & PieceBB(ROOK, Color::BLACK)) ss << gsl::narrow_cast<char>(static_cast<int>('a') + (InitialRookSquare.at(2) & 7)); else ss << 'k';
		}
		if (B0_0_0 & CastlingOptions) {
			Bitboard sideOfKing = 0ull;
			for (int i = InitialKingSquare.at(static_cast<int>(Color::BLACK)) + 1; i >= A8; --i) sideOfKing |= 1ull << i;
			sideOfKing &= ~InitialRookSquareBB.at(3);
			if (sideOfKing & PieceBB(ROOK, Color::BLACK)) ss << gsl::narrow_cast<char>(static_cast<int>('a') + (InitialRookSquare.at(3) & 7)); else ss << 'q';
		}
	}
	else {
		if (W0_0 & CastlingOptions)
			ss << 'K';
		if (W0_0_0 & CastlingOptions)
			ss << 'Q';
		if (B0_0 & CastlingOptions)
			ss << 'k';
		if (B0_0_0 & CastlingOptions)
			ss << 'q';
	}
	if (only_epd) {
		ss << (EPSquare == OUTSIDE ? " -" : " " + toString(EPSquare));
	}
	else {
		ss << (EPSquare == OUTSIDE ? " - " : " " + toString(EPSquare) + " ")
			<< int(DrawPlyCount) << " "
			//<< 1 + (_ply - (_sideToMove == BLACK)) / 2;
			<< "1";
	}
	return ss.str();
}

#pragma warning( push )
#pragma warning( disable : 26446 )
std::wstring Position::html() const
{
	std::array<std::wstring, 13> upiece = { L"\u2655", L"\u265b", L"\u2656", L"\u265c", L"\u2657", L"\u265d", L"\u2658", L"\u265e", L"\u2659", L"\u265f", L"\u2654", L"\u265a",  L"\u0020" };
	std::wstringstream wss;
	wss << L"<table style = \"text-align:center;border-spacing:0pt;font-family:'Arial Unicode MS'; border-collapse:collapse; border-color: black; border-style: solid; border-width: 0pt 0pt 0pt 0pt\">";
	wss << L"<tbody>";
	for (int row = 7; row >= 0; --row) {
		wss << "<tr>";
		wss << L"<td style = \"width:12pt\">" << row + 1 << L"</td>" << std::endl;
		for (int file = 0; file < 8; ++file) {
			const int sq = 8 * row + file;
			const Bitboard square = 1ull << sq;
			std::wstring bg = (square & DARKSQUARES) != 0 ? L" bgcolor=\"silver\"" : L"";
			wss << L"<td style=\"width:24pt; height:24pt; border-collapse:collapse; border-color: black; border-style: solid; border-width: 1pt 1pt 1pt 1pt\"" << bg << L"><span style=\"font-size:150%;\">"
				<< upiece.at(Board.at(sq)) << L"</span></td>" << std::endl;
		}
		wss << L"</tr>" << std::endl;
	}
	wss << L"<tr><td></td>";
	for (int file = 0; file < 8; ++file) {
		wss << L"<td>" << L"abcdefgh"[file] << L"</td>";
	}
	wss << L"</tr></tbody> </table>";
	return wss.str();
}
#pragma warning( pop )

std::string Position::print() {
	std::ostringstream ss;

	ss << "\n +---+---+---+---+---+---+---+---+\n";

	for (int r = 7; r >= 0; --r) {
		for (int f = 0; f <= 7; ++f)
			ss << " | " << PieceToChar.at(Board.at(8 * r + f));

		ss << " |\n +---+---+---+---+---+---+---+---+\n";
	}
	ss << "\nChecked:         " << std::boolalpha << Checked() << std::noboolalpha
		<< "\nEvaluation:      " << static_cast<int>(this->evaluate())
		<< "\nFen:             " << fen()
		<< "\nHash:            " << std::hex << std::uppercase << std::setfill('0') << std::setw(16) << Hash
		//<< "\nNormalized Hash: " << std::hex << std::uppercase << std::setfill('0') << std::setw(16) << GetNormalizedHash()
		<< "\nMaterial Key:    " << std::hex << std::uppercase << std::setfill('0') << std::setw(16) << MaterialKey
		<< "\n";
	return ss.str();
}

std::string Position::printGeneratedMoves() {
	std::ostringstream ss;
	for (int i = 0; i < movepointer - 1; ++i) {
		ss << toString(moves.at(i).move) << "\t" << static_cast<int>(moves.at(i).score) << "\n";
	}
	return ss.str();
}

MaterialKey_t Position::calculateMaterialKey() const noexcept {
	MaterialKey_t key = MATERIAL_KEY_OFFSET;
	for (int i = WQUEEN; i <= BPAWN; ++i)
		key += materialKeyFactors.at(i) * popcount(PieceBB(GetPieceType(static_cast<Piece>(i)), static_cast<Color>(i & 1)));
	return key;
}

PawnKey_t Position::calculatePawnKey() const noexcept {
	PawnKey_t key = 0;
	Bitboard pawns = PieceBB(PAWN, Color::WHITE);
	while (pawns) {
		key ^= ZobristKeys.at(WPAWN).at(lsb(pawns));
		pawns &= pawns - 1;
	}
	pawns = PieceBB(PAWN, Color::BLACK);
	while (pawns) {
		key ^= ZobristKeys.at(BPAWN).at(lsb(pawns));
		pawns &= pawns - 1;
	}
	return key;
}

Result Position::GetResult() {
	//if (!result) {
	//	bool checked = Checked();
	//	if (checked && !CheckValidMoveExists<true>()) result = MATE;
	//	else if (!checked && !CheckValidMoveExists<false>()) result = DRAW;
	//	else if (DrawPlyCount >= 100 || checkRepetition()) result = DRAW;
	//	else result = OPEN;
	//}
	if (result == Result::RESULT_UNKNOWN) {
		if (Checked()) {
			if (CheckValidMoveExists<true>()) result = Result::OPEN; else result = Result::MATE;
		}
		else {
			if (CheckValidMoveExists<false>()) result = Result::OPEN; else result = Result::DRAW;
		}
		if (result == Result::OPEN && (DrawPlyCount >= 100 || checkRepetition()))
			result = Result::DRAW;
	}
	return result;
}

DetailedResult Position::GetDetailedResult() {
	GetResult();
	if (result == Result::OPEN) return DetailedResult::NO_RESULT;
	else if (result == Result::MATE) {
		return GetSideToMove() == Color::WHITE ? DetailedResult::BLACK_MATES : DetailedResult::WHITE_MATES;
	}
	else {
		if (DrawPlyCount >= 100) return DetailedResult::DRAW_50_MOVES;
		else if (GetMaterialTableEntry()->IsTheoreticalDraw()) return DetailedResult::DRAW_MATERIAL;
		else if (!Checked() && !CheckValidMoveExists<false>()) return DetailedResult::DRAW_STALEMATE;
		else {
			//Check for 3 fold repetition
			int repCounter = 0;
			Position* prev = Previous();
			for (int i = 0; i < (std::min(pliesFromRoot + AppliedMovesBeforeRoot, int(DrawPlyCount)) >> 1); ++i) {
				prev = prev->Previous();
				if (prev->GetHash() == GetHash()) {
					repCounter++;
					if (repCounter > 1) return DetailedResult::DRAW_REPETITION;
					prev = prev->Previous();
				}
			}
		}
	}
	return DetailedResult::NO_RESULT;
}

bool Position::checkRepetition() const noexcept {
	Position* prev = Previous();
	for (int i = 0; i < (std::min(pliesFromRoot + AppliedMovesBeforeRoot, int(DrawPlyCount)) >> 1); ++i) {
		prev = prev->Previous();
		if (prev->GetHash() == GetHash())
			return true;
		prev = prev->Previous();
	}
	return false;
}

bool Position::hasRepetition() const noexcept {
	const Position* pos = this;
	while (pos && pos->GetDrawPlyCount() > 0) {
		if (pos->checkRepetition()) return true;
		pos = pos->Previous();
	}
	return false;
}

//Hashmoves, countermoves, ... aren't really reliable => therefore check if it is a valid move
bool Position::validateMove(Move move) noexcept {
	const Square fromSquare = from(move);
	const Piece movingPiece = Board.at(fromSquare);
	const Square toSquare = to(move);
	bool valid = (movingPiece != BLANK) && (GetColor(movingPiece) == SideToMove) //from field is occuppied by piece of correct color
		&& ((Board.at(toSquare) == BLANK) || (GetColor(Board.at(toSquare)) != SideToMove));
	if (valid) {
		const PieceType pt = GetPieceType(movingPiece);
		if (pt == PAWN) {
			switch (type(move)) {
			case MoveType::NORMAL:
				valid = !(ToBitboard(toSquare) & RANKS.at(7 - 7 * static_cast<int>(SideToMove))) && (((static_cast<int>(toSquare) - static_cast<int>(fromSquare)) == PawnStep() && Board.at(toSquare) == BLANK) ||
					(((static_cast<int>(toSquare) - static_cast<int>(fromSquare)) == 2 * PawnStep()) && ((fromSquare >> 3) == (1 + 5 * static_cast<int>(SideToMove))) && Board.at(toSquare) == BLANK && Board.at(toSquare - PawnStep()) == BLANK)
					|| (attacks.at(fromSquare) & OccupiedByColor.at(static_cast<int>(~SideToMove)) & ToBitboard(toSquare)));
				break;
			case MoveType::PROMOTION:
				valid = (ToBitboard(toSquare) & RANKS.at(7 - 7 * static_cast<int>(SideToMove))) &&
					(((static_cast<int>(toSquare) - static_cast<int>(fromSquare)) == PawnStep() && Board.at(toSquare) == BLANK) || (attacks.at(fromSquare) & OccupiedByColor.at(static_cast<int>(~SideToMove)) & ToBitboard(toSquare)));
				break;
			case MoveType::ENPASSANT:
				valid = toSquare == EPSquare;
				break;
			default:
				return false;
			}
		}
		else if (pt == KING && type(move) == MoveType::CASTLING) {
			valid = (CastlingOptions & CastlesbyColor.at(static_cast<int>(SideToMove))) != 0
				&& ((InBetweenFields.at(fromSquare).at(InitialRookSquare.at(2 * static_cast<int>(SideToMove) + (toSquare < fromSquare))) & OccupiedBB()) == 0)
				&& (((InBetweenFields.at(fromSquare).at(toSquare) | ToBitboard(toSquare)) & attackedByThem) == 0)
				&& (InitialRookSquareBB.at(2 * static_cast<int>(SideToMove) + (toSquare < fromSquare)) & PieceBB(ROOK, SideToMove))
				&& !Checked();
		}
		else if (type(move) == MoveType::NORMAL) valid = (attacks.at(fromSquare) & ToBitboard(toSquare)) != 0;
		else valid = false;
	}
	return valid;
}

Move Position::validMove(Move proposedMove)
{
	const ValuatedMove* vmoves = GenerateMoves<LEGAL>();
	const int movecount = GeneratedMoveCount();
	for (int i = 0; i < movecount; ++i) {
		if (vmoves[i].move == proposedMove) return proposedMove;
	}
	return movecount > 0 ? vmoves[0].move : MOVE_NONE;
}
bool Position::givesCheck(Move move) noexcept
{
	const Square fromSquare = from(move);
	Square toSquare = to(move);
	const Square kingSquare = kingSquares.at(static_cast<int>(~SideToMove));
	const MoveType moveType = type(move);
	PieceType pieceType = PieceType::NO_TYPE;
	if (moveType == MoveType::NORMAL)
		pieceType = GetPieceType(GetPieceOnSquare(fromSquare));
	else if (moveType == MoveType::PROMOTION)
		pieceType = promotionType(move);
	else if (moveType == MoveType::ENPASSANT) pieceType = PieceType::PAWN;
	else {
		toSquare = gsl::narrow_cast<File>(toSquare & 7) == File::FileG ? Square(toSquare - 1) : Square(toSquare + 1);
		pieceType = ROOK; //Castling
	}
	//Check direct checks
	switch (pieceType)
	{
	case KNIGHT:
		if (ToBitboard(toSquare) & KnightAttacks.at(kingSquare)) return true;
		break;
	case PAWN:
		if (PawnAttacks.at(static_cast<int>(SideToMove)).at(toSquare) & ToBitboard(kingSquare)) return true;
		break;
	case BISHOP:
		if (BishopTargets(toSquare, OccupiedBB() & ~ToBitboard(fromSquare)) & PieceBB(KING, ~SideToMove)) return true;
		break;
	case ROOK:
		if (RookTargets(toSquare, OccupiedBB() & ~ToBitboard(fromSquare)) & PieceBB(KING, ~SideToMove)) return true;
		break;
	case QUEEN:
		if ((RookTargets(toSquare, OccupiedBB() & ~ToBitboard(fromSquare)) & PieceBB(KING, ~SideToMove)) || (BishopTargets(toSquare, OccupiedBB() & ~ToBitboard(fromSquare)) & PieceBB(KING, ~SideToMove))) return true;
		break;
	default:
		break;
	}
	//now check for discovered check
	const Bitboard dc = PinnedPieces(~SideToMove);
	if (dc) {
		if (moveType == MoveType::ENPASSANT) {
			//in EP-captures 2 "from"-Moves have to be checked for discovered (from-square and square of captured pawn)
			if ((ToBitboard(fromSquare) & dc) != EMPTY) { //capturing pawn was pinned
				if ((ToBitboard(toSquare) & RaysBySquares.at(fromSquare).at(kingSquare)) == EMPTY) return true; //pin wasn't along capturing diagonal
			}
			const Square capturedPawnSquare = static_cast<Square>(toSquare - 8 + 16 * int(SideToMove));
			if ((ToBitboard(capturedPawnSquare) & dc) == EMPTY) return false; //captured pawn isn't pinned
			if ((ToBitboard(toSquare) & RaysBySquares.at(capturedPawnSquare).at(kingSquare)) != EMPTY) return false; //captured pawn was pinned, but capturing pawn is now blocking
			return true;
		}
		else {
			if (moveType == MoveType::CASTLING || (ToBitboard(fromSquare) & dc) == EMPTY) return false;
			return (ToBitboard(toSquare) & RaysBySquares.at(fromSquare).at(kingSquare)) == EMPTY;
		}
	}
	return false;
}

bool Position::oppositeColoredBishops() const noexcept
{
	return popcount(PieceBB(BISHOP, Color::WHITE)) == 1 && popcount(PieceBB(BISHOP, Color::BLACK)) == 1 && popcount(PieceTypeBB(BISHOP) & DARKSQUARES) == 1;
}

bool Position::validateMove(ExtendedMove move) noexcept {
	const Square fromSquare = from(move.move);
	return Board.at(fromSquare) == move.piece && validateMove(move.move);
}


void Position::NullMove(Square epsquare, Move lastApplied) noexcept {
	SwitchSideToMove();
	SetEPSquare(epsquare);
	lastAppliedMove = lastApplied;
	const Bitboard tmp = attackedByThem;
	attackedByThem = attackedByUs;
	attackedByUs = tmp;
	if (StaticEval != VALUE_NOTYETDETERMINED) {
		StaticEval = -StaticEval + 2 * settings::parameter.BONUS_TEMPO.getScore(material->Phase);
	}
}

void Position::deleteParents() {
	if (previous != nullptr) previous->deleteParents();
	delete(previous);
}

bool Position::checkMaterialIsUnusual() const noexcept {
	return popcount(PieceBB(QUEEN, Color::WHITE)) > 1
		|| popcount(PieceBB(QUEEN, Color::BLACK)) > 1
		|| popcount(PieceBB(ROOK, Color::WHITE)) > 2
		|| popcount(PieceBB(ROOK, Color::BLACK)) > 2
		|| popcount(PieceBB(BISHOP, Color::WHITE)) > 2
		|| popcount(PieceBB(BISHOP, Color::BLACK)) > 2
		|| popcount(PieceBB(KNIGHT, Color::WHITE)) > 2
		|| popcount(PieceBB(KNIGHT, Color::BLACK)) > 2;
}

ValuatedMove* Position::GenerateForks(bool withChecks) noexcept
{
	movepointer -= (movepointer != 0);
	ValuatedMove* firstMove = &moves.data()[movepointer];
	Bitboard knights = PieceBB(KNIGHT, SideToMove);
	Bitboard targets = EMPTY;
	Bitboard forkTargets = EMPTY;
	if (withChecks) {
		targets = ~OccupiedBB() & ~attackedByThem;
		forkTargets = PieceBB(QUEEN, ~SideToMove) | PieceBB(ROOK, ~SideToMove) | PieceBB(KING, ~SideToMove);
		forkTargets |= ColorBB(~SideToMove) & ~attackedByThem;
	}
	else {
		targets = ~OccupiedBB() & ~attackedByThem & ~KnightAttacks.at(kingSquares.at(static_cast<int>(~SideToMove)));
		forkTargets = PieceBB(QUEEN, ~SideToMove) | PieceBB(ROOK, ~SideToMove);
		forkTargets |= ColorBB(~SideToMove) & ~attackedByThem & ~PieceTypeBB(KING);
	}
	const int forkTargetCount = popcount(forkTargets);
	if (forkTargetCount > 1) {
		while (knights) {
			const Square fromSquare = lsb(knights);
			Bitboard knightTargets = KnightAttacks.at(fromSquare) & targets;
			while (knightTargets) {
				const Square toSquare = lsb(knightTargets);
				if (popcount(KnightAttacks.at(toSquare) & forkTargets) > 1) {
					AddMove(createMove(fromSquare, toSquare));
				}
				knightTargets &= knightTargets - 1;
			}
			knights &= knights - 1;
		}
	}
	AddNullMove();
	return firstMove;
}

bool Position::mateThread() const noexcept
{
	const Bitboard bbEscapeSquares = GetAttacksFrom(kingSquares.at(static_cast<int>(~SideToMove))) & ~ColorBB(~SideToMove) & ~attackedByUs;
	const int countEscapeSquares = popcount(bbEscapeSquares);
	return (countEscapeSquares <= 1); //At most one escape square
		//|| (countEscapeSquares == 2  && (bbEscapeSquares & (Rank1 | Rank8)) == bbEscapeSquares); //Backrank mate
}

#pragma warning( push )
#pragma warning( disable : 26446 )
std::string Position::toSan(Move move) {
	const Square toSquare = to(move);
	const Square fromSquare = from(move);
	if (type(move) == MoveType::CASTLING) {
		if (toSquare > fromSquare) return "O-O"; else return "O-O-O";
	}
	const PieceType pt = GetPieceType(Board.at(from(move)));
	const bool isCapture = (Board.at(to(move)) != BLANK) || (type(move) == MoveType::PROMOTION);
	if (pt == PAWN) {
		if (isCapture || type(move) == MoveType::ENPASSANT) {
			if (type(move) == MoveType::PROMOTION) {
				const char ch[] = { toChar(gsl::narrow_cast<File>(fromSquare & 7)), 'x', toChar(gsl::narrow_cast<File>(toSquare & 7)), toChar(gsl::narrow_cast<Rank>(toSquare >> 3)), '=', "QRBN"[promotionType(move)], 0 };
				return std::string(ch);
			}
			else {
				const char ch[] = { toChar(gsl::narrow_cast<File>(fromSquare & 7)), 'x', toChar(gsl::narrow_cast<File>(toSquare & 7)), toChar(gsl::narrow_cast<Rank>(toSquare >> 3)), 0 };
				return std::string(ch);
			}
		}
		else {
			if (type(move) == MoveType::PROMOTION) {
				const char ch[] = { toChar(gsl::narrow_cast<File>(toSquare & 7)), toChar(gsl::narrow_cast<Rank>(toSquare >> 3)), '=', "QRBN"[promotionType(move)], 0 };
				return std::string(ch);
			}
			else {
				const char ch[] = { toChar(gsl::narrow_cast<File>(toSquare & 7)), toChar(gsl::narrow_cast<Rank>(toSquare >> 3)), 0 };
				return std::string(ch);
			}
		}
	}
	else if (pt == KING) {
		if (isCapture) {
			const char ch[] = { 'K', 'x', toChar(gsl::narrow_cast<File>(toSquare & 7)), toChar(gsl::narrow_cast<Rank>(toSquare >> 3)), 0 };
			return std::string(ch);
		}
		else {
			const char ch[] = { 'K', toChar(gsl::narrow_cast<File>(toSquare & 7)), toChar(gsl::narrow_cast<Rank>(toSquare >> 3)), 0 };
			return std::string(ch);
		}
	}
	//Check if diambiguation is needed
	Move dMove = MOVE_NONE;
	ValuatedMove* legalMoves = GenerateMoves<LEGAL>();
	while (legalMoves->move) {
		if (legalMoves->move != move && to(legalMoves->move) == toSquare && GetPieceType(Board.at(from(legalMoves->move))) == pt) {
			dMove = legalMoves->move;
			break;
		}
		legalMoves++;
	}
	char ch[6];
	ch[0] = "QRBN"[pt];
	int indx = 1;
	if (dMove != MOVE_NONE) {
		if ((from(dMove) & 7) == (fromSquare & 7)) ch[indx] = toChar(gsl::narrow_cast<Rank>(from(dMove) >> 3)); else ch[indx] = toChar(gsl::narrow_cast<File>(from(dMove) & 7));
		indx++;
	}
	if (isCapture) {
		ch[indx] = 'x';
		indx++;
	}
	ch[indx] = toChar(gsl::narrow_cast<File>(toSquare & 7));
	indx++;
	ch[indx] = toChar(gsl::narrow_cast<Rank>(toSquare >> 3));
	indx++;
	ch[indx] = 0;
	return ch;
}
#pragma warning( pop )

Move Position::parseSan(std::string move) {
	const ValuatedMove* legalMoves = GenerateMoves<LEGAL>();
	while (legalMoves->move) {
		if (move.find(toSan(legalMoves->move)) != std::string::npos) return legalMoves->move;
	}
	return MOVE_NONE;
}

std::string Position::printEvaluation() {
	if (material->EvaluationFunction == &evaluateDefault) {
		return printDefaultEvaluation(*this);
	}
	else {
		std::stringstream ss;
		ss << "Special Evaluation Function used!" << std::endl;
		Value score = evaluate();
		if (SideToMove == Color::BLACK) score = -score;
		ss << "Total evaluation: " << static_cast<int>(score) << " (white side)" << std::endl;
		return ss.str();
	}
}

std::string Position::printDbgEvaluation() {
	if (material->EvaluationFunction == &evaluateDefault) {
		return printDbgDefaultEvaluation(*this);
	}
	else {
		std::stringstream ss;
		ss << evaluate();
		return ss.str();
	}
}

void Position::CalculatePinnedPieces() noexcept
{
	for (int colorOfKing = 0; colorOfKing < 2; ++colorOfKing) {
		bbPinned.at(colorOfKing) = EMPTY;
		bbPinner.at(colorOfKing) = EMPTY;
		const Square kingSquare = kingSquares.at(colorOfKing);
		Bitboard pinner = (OccupiedByPieceType.at(ROOK) | OccupiedByPieceType.at(QUEEN)) & SlidingAttacksRookTo.at(kingSquare);
		pinner |= (OccupiedByPieceType.at(BISHOP) | OccupiedByPieceType.at(QUEEN)) & SlidingAttacksBishopTo.at(kingSquare);
		pinner &= OccupiedByColor.at(colorOfKing ^ 1);
		const Bitboard occ = OccupiedBB();
		while (pinner)
		{
			const Bitboard blocker = InBetweenFields.at(lsb(pinner)).at(kingSquare) & occ;
			if (popcount(blocker) == 1) bbPinned.at(colorOfKing) |= blocker;
			bbPinner.at(colorOfKing) |= isolateLSB(pinner);
			pinner &= pinner - 1;
		}
	}
}
