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

#include <vector>
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <iomanip>
#include "board.h"
#include "types.h"
#include "material.h"
#include "position.h"
#include "hashtables.h"

#ifdef STRACE
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
auto file_logger = spdlog::basic_logger_mt("basic_logger", "log.txt");
#endif

bool Chess960 = false;

void InitializeDistance() noexcept {
	for (int sq1 = 0; sq1 < 64; sq1++) {
		for (int sq2 = 0; sq2 < 64; sq2++) {
			Distance.at(sq1).at(sq2) = gsl::narrow_cast<uint8_t>( std::max(abs((sq1 >> 3) - (sq2 >> 3)), abs((sq1 & 7) - (sq2 & 7))) );
		}
	}
}

void InitializeSquareBB() noexcept {
	for (int sq1 = 0; sq1 < 64; sq1++) SquareBB.at(sq1) = 1ull << sq1;
};

void InitializeInBetweenFields() noexcept {
	for (int from = 0; from < 64; from++) {
		InBetweenFields.at(from).at(from) = 0ull;
		for (int to = from + 1; to < 64; to++) {
			InBetweenFields.at(from).at(to) = 0;
			const int colFrom = from & 7;
			const int colTo = to & 7;
			const int rowFrom = from >> 3;
			int rowTo = to >> 3;
			if (colFrom == colTo) {
				for (int row = rowFrom + 1; row < rowTo; row++)
					InBetweenFields.at(from).at(to) = InBetweenFields.at(from).at(to)
					| (1ull << (8 * row + colTo));
			}
			else if (rowFrom == rowTo) {
				for (int col = colFrom + 1; col < colTo; col++)
					InBetweenFields.at(from).at(to) = InBetweenFields.at(from).at(to)
					| (1ull << (8 * rowTo + col));
			}
			else if (((to - from) % 7) == 0 && rowTo > rowFrom
				&& colTo < colFrom) //long diagonal A1-H8 has difference 63 which results in %7 = 0
			{
				for (int i = from + 7; i < to; i = i + 7)
					InBetweenFields.at(from).at(to) = InBetweenFields.at(from).at(to)
					| (1ull << i);
			}
			else if (((to - from) % 9) == 0 && rowTo > rowFrom && colTo > colFrom) {
				for (int i = from + 9; i < to; i = i + 9)
					InBetweenFields.at(from).at(to) = InBetweenFields.at(from).at(to)
					| (1ull << i);
			}
		}
	}
	for (int from = 0; from < 64; from++) {
		for (int to = from + 1; to < 64; to++) {
			InBetweenFields.at(to).at(from) = InBetweenFields.at(from).at(to);
		}
	}
}

void InitializeKnightAttacks() noexcept {
	const std::array<int, 8> knightMoves = { -17, -15, -10, -6, 6, 10, 15, 17 };
	for (int square = A1; square <= H8; square++) {
		KnightAttacks.at(square) = 0;
		const int col = square & 7;
		for (int move = 0; move < 8; move++)
		{
			const int to = square + knightMoves.at(move);
			if (to < 0 || to > 63) continue;
			const int toCol = to & 7;
			const int colldiff = col - toCol;
			if (colldiff < -2 || colldiff > 2) continue;
			KnightAttacks.at(square) |= ToBitboard(to);
		}
	}
}

void InitializeKingAttacks() noexcept {
	const std::array<int, 8> kingMoves = { -9, -8, -7, -1, 1, 7, 8, 9 };
	for (int square = 0; square < 64; square++)
	{
		KingAttacks.at(square) = 0;
		const int col = square & 7;
		for (int move = 0; move < 8; move++)
		{
			const int to = square + kingMoves.at(move);
			if (to < 0 || to > 63) continue;
			const int toCol = to & 7;
			const int colldiff = col - toCol;
			if (colldiff < -1 || colldiff > 1) continue;
			KingAttacks.at(square) |= ToBitboard(to);
		}
	}
}

void InitializePawnAttacks() noexcept {
	for (Square sq = A2; sq <= H7; ++sq) {
		Bitboard sqBB = ToBitboard(sq);
		PawnAttacks.at(static_cast<int>(Color::WHITE)).at(sq) = (sqBB << 7) & NOT_H_FILE;
		PawnAttacks.at(static_cast<int>(Color::WHITE)).at(sq) |= (sqBB << 9) & NOT_A_FILE;
		PawnAttacks.at(static_cast<int>(Color::BLACK)).at(sq) = (sqBB >> 9) & NOT_H_FILE;
		PawnAttacks.at(static_cast<int>(Color::BLACK)).at(sq) |= (sqBB >> 7) & NOT_A_FILE;
	}
}

void InitializeSlidingAttacksTo() noexcept {
	for (int row = 0; row < 8; ++row) {
		for (int col = 0; col < 8; ++col) {
			int square = 8 * row + col;
			SlidingAttacksRookTo.at(square) = RANKS.at(row) | FILES.at(col);
			SlidingAttacksRookTo.at(square) &= ~(1ull << square);
			//Now the diagonals
			SlidingAttacksBishopTo.at(square) = 0ull;
			for (int target = square + 9; target < 64; target += 9) {
				if (target > 63) break;
				if ((target & 7) == 0) break;
				SlidingAttacksBishopTo.at(square) |= 1ull << target;
			}
			for (int target = square + 7; target < 64; target += 7) {
				if (target > 63) break;
				if ((target & 7) == 7) break;
				SlidingAttacksBishopTo.at(square) |= 1ull << target;
			}
			for (int target = square - 9; target >= 0; target -= 9) {
				if (target < 0) break;
				if ((target & 7) == 7) break;
				SlidingAttacksBishopTo.at(square) |= 1ull << target;
			}
			for (int target = square - 7; target >= 0; target -= 7) {
				if (target < 0) break;
				if ((target & 7) == 0) break;
				SlidingAttacksBishopTo.at(square) |= 1ull << target;
			}
		}
	}
}

void Ray2RayBySquares(Bitboard ray) noexcept {
	Bitboard b1 = ray;
	while (b1) {
		const Square s1 = lsb(b1);
		b1 &= b1 - 1;
		Bitboard b2 = b1;
		while (b2) {
			const Square s2 = lsb(b2);
			b2 &= b2 - 1;
			RaysBySquares.at(s1).at(s2) = RaysBySquares.at(s2).at(s1) = ray;
		}
	}
}

void InitializeRaysBySquares() noexcept {
	for (int i = 0; i < 64; ++i) {
		for (int j = 0; j < 64; ++j) {
			RaysBySquares.at(i).at(j) = 0ull;
		}
	}
	for (int i = 0; i < 8; i++) Ray2RayBySquares(FILES.at(i));
	for (int i = 0; i < 8; i++) Ray2RayBySquares(RANKS.at(i));
	for (int i = 0; i < 13; i++) Ray2RayBySquares(Diagonals.at(i));
	for (int i = 0; i < 13; i++) Ray2RayBySquares(AntiDiagonals.at(i));
}

//Bitboard AffectedBy[64];
//void InitializeAffectedBy() {
//	for (int row = 0; row < 8; ++row) {
//		for (int col = 0; col < 8; ++col) {
//			int square = 8 * row + col;
//			AffectedBy[square] = 1ull << square;
//			if ((col == 0 || col == 7) && (row == 0 || row == 7)) continue; //Corner squares don't affect any other attack
//			if (row == 0) { AffectedBy[square] = RANK1; continue; } //Squares on border only affect the other squares on the same border
//			if (row == 7) { AffectedBy[square] = RANK8; continue; }
//			if (col == 0)  { AffectedBy[square] = A_FILE; continue; }
//			if (col == 7)  { AffectedBy[square] = H_FILE; continue; }
//			//Remaining fields (all non-border squares affect fields in all 8 directions
//			AffectedBy[square] = RANKS.at(row) | FILES.at(col); 
//			//Now the diagonals
//			for (int target = square + 9; target < 64; target += 9) {
//				if (target > 63) break;
//				if ((target & 7) == 0) break;
//				AffectedBy[square] |= 1ull << target;
//			}
//			for (int target = square + 7; target < 64; target += 7) {
//				if (target > 63) break;
//				if ((target & 7) == 7) break;
//				AffectedBy[square] |= 1ull << target;
//			}
//			for (int target = square -9; target >= 0; target -= 9) {
//				if (target < 0) break;
//				if ((target & 7) == 7) break;
//				AffectedBy[square] |= 1ull << target;
//			}
//			for (int target = square -7; target >= 0; target -= 7) {
//				if (target < 0) break;
//				if ((target & 7) == 0) break;
//				AffectedBy[square] |= 1ull << target;
//			}
//		}
//	}
//
//}

//ShadowedFields[s1][s2] contains the fields which are seen from s1 shadowed by a piece on s2
//f.e. ShadowedFields[A1][F1] = contains G1 and H1
void InitializeShadowedFields() noexcept
{
	for (int square = 0; square < 64; square++)
	{
		for (int i = 0; i < 64; i++) ShadowedFields.at(square).at(i) = 0;
	}
	for (int square = 0; square < 64; square++)
	{
		const int col = square & 7;
		//North
		for (int blocker = square + 8; blocker <= 55; blocker += 8)
		{
			Bitboard mask = 0;
			for (int i = blocker + 8; i <= 63; i += 8) mask |= ToBitboard(i);
			ShadowedFields.at(square).at(blocker) = mask;
		}
		//South
		for (int blocker = square - 8; blocker >= 8; blocker -= 8)
		{
			Bitboard mask = 0;
			for (int i = blocker - 8; i >= 0; i -= 8) mask |= ToBitboard(i);
			ShadowedFields.at(square).at(blocker) = mask;
		}
		//East
		if (col < 6)
		{
			for (int blocker = square + 1; (blocker & 7) > col; blocker++)
			{
				Bitboard mask = 0;
				for (int i = blocker + 1; (i & 7) > (blocker & 7); i++) mask |= ToBitboard(i);
				ShadowedFields.at(square).at(blocker) = mask;
			}
		}
		//West
		if (col > 1)
		{
			for (int blocker = square - 1; (blocker & 7) < col; blocker--)
			{
				Bitboard mask = 0;
				for (int i = blocker - 1; (i & 7) < (blocker & 7); i--) mask |= ToBitboard(i);
				ShadowedFields.at(square).at(blocker) = mask;
			}
		}
		//NorthEast
		if (col < 6 && square < 48)
		{
			for (int blocker = square + 9; ((blocker & 7) > col) && blocker < 55; blocker += 9)
			{
				Bitboard mask = 0;
				for (int i = blocker + 9; (i & 7) > col && i <= 63; i += 9) mask |= ToBitboard(i);
				ShadowedFields.at(square).at(blocker) = mask;
			}
		}
		//NorthWest
		if (col > 1 && square < 48)
		{
			for (int blocker = square + 7; ((blocker & 7) < col) && blocker < 55; blocker += 7)
			{
				Bitboard mask = 0;
				for (int i = blocker + 7; (i & 7) < col && i <= 63; i += 7) mask |= ToBitboard(i);
				ShadowedFields.at(square).at(blocker) = mask;
			}
		}
		//SouthEast
		if (col < 6 && square > 15)
		{
			for (int blocker = square - 7; ((blocker & 7) > col) && blocker > 7; blocker -= 7)
			{
				Bitboard mask = 0;
				for (int i = blocker - 7; (i & 7) > col && i >= 0; i -= 7) mask |= ToBitboard(i);
				ShadowedFields.at(square).at(blocker) = mask;
			}
		}
		//SouthWest
		if (col > 1 && square > 15)
		{
			for (int blocker = square - 9; ((blocker & 7) < col) && blocker > 7; blocker -= 9)
			{
				Bitboard mask = 0;
				for (int i = blocker - 9; (i & 7) < col && i >= 0; i -= 9) mask |= ToBitboard(i);
				ShadowedFields.at(square).at(blocker) = mask;
			}
		}
	}
}

#ifdef USE_PEXT

void initializePextMasks() {
	for (int rank = 0; rank < 8; ++rank) {
		for (int file = 0; file < 8; ++file) {
			int square = 8 * rank + file;
			Bitboard squareBB = 1ull << square;
			ROOK_MASKS[square] = RANKS.at(rank) | FILES.at(file);
			for (int edge = 0; edge < 4; ++edge) {
				if (!(EDGE[edge] & squareBB))
					ROOK_MASKS[square] &= ~EDGE[edge];
			}
			ROOK_MASKS[square] &= ~squareBB;
			BISHOP_MASKS[square] = 0ull;
			for (int i = 1; i < 8; ++i) {
				int to = square + 9 * i;
				Bitboard toBB = 1ull << to;
				if (to > 63 || (toBB & A_FILE))
					break;
				BISHOP_MASKS[square] |= toBB;
			}
			for (int i = 1; i < 8; ++i) {
				int to = square + 7 * i;
				Bitboard toBB = 1ull << to;
				if (to > 63 || (toBB & H_FILE))
					break;
				BISHOP_MASKS[square] |= toBB;
			}
			for (int i = 1; i < 8; ++i) {
				int to = square - 9 * i;
				Bitboard toBB = 1ull << to;
				if (to < 0 || (toBB & H_FILE))
					break;
				BISHOP_MASKS[square] |= toBB;
			}
			for (int i = 1; i < 8; ++i) {
				int to = square - 7 * i;
				Bitboard toBB = 1ull << to;
				if (to < 0 || (toBB & A_FILE))
					break;
				BISHOP_MASKS[square] |= toBB;
			}
			BISHOP_MASKS[square] &= ~BORDER;
		}
	}
}

void initializePextAttacks() {
	int offset = 0;
	int maxIndex = 0;
	for (int square = 0; square < 64; ++square) {
		ROOK_OFFSETS[square] = offset;
		//Traverse all subsets
		Bitboard occupany = 0ull;
		Bitboard bbMask = ROOK_MASKS[square];
		do {
			int index = (int)pext(occupany, bbMask) + offset;
			if (index > maxIndex)
				maxIndex = index;
			Bitboard attacks = 0ull;
			for (int i = 1; i < 8; ++i) {
				int to = square + i * 8;
				if (to > 63)
					break;
				Bitboard toBB = 1ull << to;
				attacks |= toBB;
				if (occupany & toBB)
					break;
			}
			for (int i = 1; i < 8; ++i) {
				int to = square - i * 8;
				if (to < 0)
					break;
				Bitboard toBB = 1ull << to;
				attacks |= toBB;
				if (occupany & toBB)
					break;
			}
			for (int i = 1; i < 8; ++i) {
				int to = square + i;
				Bitboard toBB = 1ull << to;
				if (to > 63 || (toBB & A_FILE))
					break;
				attacks |= toBB;
				if (occupany & toBB)
					break;
			}
			for (int i = 1; i < 8; ++i) {
				int to = square - i;
				Bitboard toBB = 1ull << to;
				if (to < 0 || (toBB & H_FILE))
					break;
				attacks |= toBB;
				if (occupany & toBB)
					break;
			}
			ATTACKS[index] = attacks;
			occupany = (occupany - bbMask) & bbMask;
		} while (occupany);
		offset = maxIndex + 1;
	}
	//Same procedure for Bishops
	for (int square = 0; square < 64; ++square) {
		BISHOP_OFFSETS[square] = offset;
		Bitboard bbMask = BISHOP_MASKS[square];
		//Traverse all subsets
		Bitboard occupany = 0ull;
		do {
			int index = (int)pext(occupany, bbMask) + offset;
			if (index > maxIndex)
				maxIndex = index;
			Bitboard attacks = 0ull;
			for (int i = 1; i < 8; ++i) {
				int to = square + i * 9;
				Bitboard toBB = 1ull << to;
				if (to > 63 || (toBB & A_FILE))
					break;
				attacks |= toBB;
				if (occupany & toBB)
					break;
			}
			for (int i = 1; i < 8; ++i) {
				int to = square + i * 7;
				Bitboard toBB = 1ull << to;
				if (to > 63 || (toBB & H_FILE))
					break;
				attacks |= toBB;
				if (occupany & toBB)
					break;
			}
			for (int i = 1; i < 8; ++i) {
				int to = square - i * 9;
				Bitboard toBB = 1ull << to;
				if (to < 0 || (toBB & H_FILE))
					break;
				attacks |= toBB;
				if (occupany & toBB)
					break;
			}
			for (int i = 1; i < 8; ++i) {
				int to = square - i * 7;
				Bitboard toBB = 1ull << to;
				if (to < 0 || (toBB & A_FILE))
					break;
				attacks |= toBB;
				if (occupany & toBB)
					break;
			}
			ATTACKS[index] = attacks;
			occupany = (occupany - bbMask) & bbMask;
		} while (occupany);
		offset = maxIndex + 1;
	}
}

void initializePext() {
	initializePextMasks();
	initializePextAttacks();
}
#else

void InitializeOccupancyMasks() noexcept {
	for (int bitRef = 0; bitRef <= 63; bitRef++) {
		Bitboard mask = 0;
		for (int i = bitRef + 8; i <= 55; i += 8)
			mask |= (1ull << i);
		for (int i = bitRef - 8; i >= 8; i -= 8)
			mask |= (1ull << i);
		for (int i = bitRef + 1; (i & 7) != 7 && (i & 7) != 0; i++)
			mask |= (1ull << i);
		for (int i = bitRef - 1; (i & 7) != 7 && (i & 7) != 0 && i >= 0; i--)
			mask |= (1ull << i);
		OccupancyMaskRook.at(bitRef) = mask;
	}
	for (int bitRef = 0; bitRef <= 63; bitRef++) {
		Bitboard mask = 0;
		for (int i = bitRef + 9; (i & 7) != 7 && (i & 7) != 0 && i <= 55; i += 9)
			mask |= (1ull << i);
		for (int i = bitRef - 9; (i & 7) != 7 && (i & 7) != 0 && i >= 8; i -= 9)
			mask |= (1ull << i);
		for (int i = bitRef + 7; (i & 7) != 7 && (i & 7) != 0 && i <= 55; i += 7)
			mask |= (1ull << i);
		for (int i = bitRef - 7; (i & 7) != 7 && (i & 7) != 0 && i >= 8; i -= 7)
			mask |= (1ull << i);
		OccupancyMaskBishop.at(bitRef) = mask;
	}
}

std::array<std::vector<Bitboard>, 64> occupancyVariation;
std::array<std::vector<Bitboard>, 64> occupancyAttackSet;

void generateOccupancyVariations(bool isRook)
{
	for (int i = 0; i < 64; i++) {
		occupancyAttackSet.at(i).clear();
		occupancyVariation.at(i).clear();
	}
	std::array<Bitboard, 64> occupancies = isRook ? OccupancyMaskRook : OccupancyMaskBishop;
	for (int square = A1; square <= H8; square++) {
		const Bitboard occupancy = occupancies.at(square);
		Bitboard subset = 0;
		do {
			occupancyVariation.at(square).push_back(subset);
			//Now calculate attack set: subset without shadowed fields
			Bitboard attackset = 0;
			if (subset) {
				Bitboard temp = subset;
				do {
					const Square to = lsb(temp);
					if ((InBetweenFields.at(square).at(to) & subset) == 0) attackset |= ToBitboard(to);
					temp &= temp - 1;
				} while (temp);
			}
			else attackset = occupancy;
			occupancyAttackSet.at(square).push_back(attackset);
			subset = (subset - occupancy) & occupancy;
		} while (subset);
	}
}

std::array<int, 64> MaxIndexRook;
std::array<int, 64> MaxIndexBishop;

void InitializeMaxIndices() noexcept {
	for (size_t i = 0; i < 64; i++)
	{
		MaxIndexRook.at(i) = 1 << (64 - RookShift.at(i));
		MaxIndexBishop.at(i) = 1 << (64 - BishopShift.at(i));
		if (i == 0) {
			IndexOffsetRook.at(0) = 0;
			IndexOffsetBishop.at(0) = 0;
		}
		else {
			IndexOffsetRook.at(i) = IndexOffsetRook.at(i - 1) + MaxIndexRook.at(i - 1);
			IndexOffsetBishop.at(i) = IndexOffsetBishop.at(i - 1) + MaxIndexBishop.at(i - 1);
		}
	}
}

#pragma warning( push )
#pragma warning( disable : 26446 )
void InitializeMoveDB(bool isRook) {
	for (int square = A1; square <= H8; square++) {
		for (unsigned int i = 0; i < occupancyVariation.at(square).size(); i++) {
			if (isRook) {
				int magicIndex = gsl::narrow_cast<int>( (occupancyVariation.at(square).at(i) * RookMagics.at(square)) >> RookShift.at(square) );
				Bitboard attacks = 0;
				for (int to = square + 8; to <= H8; to += 8) {
					attacks |= ToBitboard(to);
					if ((occupancyVariation.at(square).at(i) & ToBitboard(to)) != 0) break;
				}
				for (int to = square - 8; to >= A1; to -= 8) {
					attacks |= ToBitboard(to);
					if ((occupancyVariation.at(square).at(i) & ToBitboard(to)) != 0) break;
				}
				for (int to = square + 1; (to & 7) != 0; to++) {
					attacks |= ToBitboard(to);
					if ((occupancyVariation.at(square).at(i) & ToBitboard(to)) != 0) break;
				}
				for (int to = square - 1; (to & 7) != 7; to--) {
					attacks |= ToBitboard(to);
					if ((occupancyVariation.at(square).at(i) & ToBitboard(to)) != 0) break;
				}
				MagicMovesRook[IndexOffsetRook.at(square) + magicIndex] = attacks;
			}
			else {
				int magicIndex = gsl::narrow_cast<int>( (occupancyVariation.at(square).at(i) * BishopMagics.at(square)) >> BishopShift.at(square) );
				Bitboard attacks = 0;
				for (int to = square + 9; to <= H8 && (to & 7) != 0; to += 9) {
					attacks |= ToBitboard(to);
					if ((occupancyVariation.at(square).at(i) & ToBitboard(to)) != 0) break;
				}
				for (int to = square + 7; to <= H8 && (to & 7) != 7; to += 7) {
					attacks |= ToBitboard(to);
					if ((occupancyVariation.at(square).at(i) & ToBitboard(to)) != 0) break;
				}
				for (int to = square - 7; to >= A1 && (to & 7) != 0; to -= 7) {
					attacks |= ToBitboard(to);
					if ((occupancyVariation.at(square).at(i) & ToBitboard(to)) != 0) break;
				}
				for (int to = square - 9; to >= A1 && (to & 7) != 7; to -= 9) {
					attacks |= ToBitboard(to);
					if ((occupancyVariation.at(square).at(i) & ToBitboard(to)) != 0) break;
				}
				MagicMovesBishop[IndexOffsetBishop.at(square) + magicIndex] = attacks;
			}
		}
	}
}
#pragma warning( pop )

void InitializeMagic() {
	InitializeMaxIndices();
	InitializeOccupancyMasks();
	generateOccupancyVariations(false);
	InitializeMoveDB(false);
	generateOccupancyVariations(true);
	InitializeMoveDB(true);
}

#endif

void Initialize(bool quiet) {
#ifdef STRACE
	spdlog::set_pattern("%v");
	spdlog::set_default_logger(file_logger);
#endif
	Chess960 = false;
	const int64_t begin = now();
	InitializeSquareBB();
	InitializeDistance();
	InitializeInBetweenFields();
	InitializeKingAttacks();
	InitializeKnightAttacks();
	InitializePawnAttacks();
	//	InitializeAffectedBy();
	InitializeSlidingAttacksTo();
	InitializeRaysBySquares();
#ifdef USE_PEXT
	initializePext();
#else
	InitializeMagic();
#endif
	InitializeMaterialTable();
	InitializeShadowedFields();
	pawn::initialize();
	tt::InitializeTranspositionTable();
	const int64_t end = now();
	const auto runtime = end - begin;
	if (!quiet) std::cout << "Initialization Time: " << runtime << "ms" << std::endl;
}

#ifdef _DEBUG
std::string printMove(Move move)
{
	return toString(move);
}
#endif

Move parseMoveInUCINotation(const std::string& uciMove, const Position& pos) {
	const Square fromSquare = gsl::narrow_cast<Square>(uciMove.at(0) - 'a' + 8 * (uciMove.at(1) - '1'));
	const Square toSquare = gsl::narrow_cast<Square>(uciMove.at(2) - 'a' + 8 * (uciMove.at(3) - '1'));
	if (uciMove.length() > 4) {
		switch (uciMove.at(4)) {
		case 'q': case 'Q':
			return createMove<MoveType::PROMOTION>(fromSquare, toSquare, QUEEN);
		case 'r': case 'R':
			return createMove<MoveType::PROMOTION>(fromSquare, toSquare, ROOK);
		case 'b': case 'B':
			return createMove<MoveType::PROMOTION>(fromSquare, toSquare, BISHOP);
		case 'n': case 'N':
			return createMove<MoveType::PROMOTION>(fromSquare, toSquare, KNIGHT);
		default:
			assert(false);
		}
	}
	if (Chess960 && GetPieceType(pos.GetPieceOnSquare(fromSquare)) == KING && pos.GetPieceOnSquare(toSquare) == GetPiece(ROOK, pos.GetSideToMove())) {
		if (fromSquare < toSquare) {
			return createMove<MoveType::CASTLING>(InitialKingSquare.at(static_cast<size_t>(pos.GetSideToMove())), InitialRookSquare.at(2 * static_cast<int>(pos.GetSideToMove())));
		}
		else {
			return createMove<MoveType::CASTLING>(InitialKingSquare.at(static_cast<size_t>(pos.GetSideToMove())), InitialRookSquare.at(2 * static_cast<int>(pos.GetSideToMove()) + 1));
		}
	}
	else {
		if ((fromSquare == E1 && pos.GetPieceOnSquare(fromSquare) == WKING
			&& (toSquare == G1 || toSquare == C1))
			|| (fromSquare == E8 && pos.GetPieceOnSquare(fromSquare) == BKING
				&& (toSquare == G8 || toSquare == C8))) {
			return createMove<MoveType::CASTLING>(fromSquare, toSquare);
		}
	}
	if (toSquare == pos.GetEPSquare() && GetPieceType(pos.GetPieceOnSquare(fromSquare)) == PAWN) return createMove<MoveType::ENPASSANT>(fromSquare, toSquare);
	return createMove(fromSquare, toSquare);
}