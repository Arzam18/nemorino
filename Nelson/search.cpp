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

#include <stdlib.h>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <math.h>
#include <cstring>
#include <thread>
#include <chrono>
#include "search.h"
#include "hashtables.h"
#include "evaluation.h"
#include "tbprobe.h"

void Search::Reset() noexcept {
	while (thread_pool != nullptr && Stop.load() && thread_pool->tasks_active() > 0);
	BestMove.move = MOVE_NONE;
	BestMove.score = VALUE_ZERO;
	NodeCount = 0;
	QNodeCount = 0;
	MaxDepth = 0;
	PonderMode.store(false);
	threadLocalData.History.age();
	searchMoves.clear();
	threadLocalData.cmHistory.age();
	threadLocalData.followupHistory.age();
	threadLocalData.killerManager.clear();
	for (int i = 0; i < PV_MAX_LENGTH; ++i) PVMoves.at(i) = MOVE_NONE;
}

void Search::NewGame()  noexcept {
	Reset();
	for (int i = 0; i < 12; ++i) {
		for (int j = 0; j < 64; ++j)counterMove.at(i).at(j) = MOVE_NONE;
	}
	threadLocalData.cmHistory.initialize();
	threadLocalData.History.initialize();
	threadLocalData.followupHistory.initialize();
}

std::string Search::PrincipalVariation(Position& pos, int depth) {
	std::stringstream ss;
	int i = 0;
	ponderMove = MOVE_NONE;
	//First get PV from PV array...
	for (; i < depth && i < PV_MAX_LENGTH; ++i) {
		if (PVMoves.at(i) == MOVE_NONE || !pos.validateMove(PVMoves.at(i))) break;
		Position next(pos);
		if (!next.ApplyMove(PVMoves.at(i))) break;
		pos = next;
		if (i > 0) ss << " ";
		ss << toString(PVMoves.at(i));
		if (i == 1) ponderMove = PVMoves.at(i);
	}
	//...then continue with moves from transposition table
	for (; i < depth && i < PV_MAX_LENGTH; ++i) {
		const Move hashmove = settings::parameter.HelperThreads == 0 ? tt::hashmove<tt::UNSAFE>(pos.GetHash()) : tt::hashmove<tt::THREAD_SAFE>(pos.GetHash());
		if (hashmove == MOVE_NONE || !pos.validateMove(hashmove)) break;
		Position next(pos);
		if (!next.ApplyMove(hashmove)) break;
		pos = next;
		if (i > 0) ss << " ";
		ss << toString(hashmove);
		if (i == 1) ponderMove = hashmove;
	}
	return ss.str();
}

//Creates the "thinking output" while running in UCI or XBoard mode
void Search::info(Position& pos, int pvIndx, SearchResultType srt) {
	if (UciOutput) {
		Position npos(pos);
		npos.copy(pos);
		std::string srtString;
		if (srt == SearchResultType::FAIL_LOW) srtString = " upperbound"; else if (srt == SearchResultType::FAIL_HIGH) srtString = " lowerbound";
		const uint64_t effectiveTBHits = tbHits > 1 ? tbHits * (settings::parameter.HelperThreads + 1) : tbHits;
		if (abs(static_cast<int>(BestMove.score)) <= static_cast<int>(VALUE_MATE_THRESHOLD))
			sync_cout << "info depth " << _depth.load() << " seldepth " << std::max(MaxDepth, _depth.load()) << " multipv " << pvIndx + 1 << " score cp " << static_cast<int>(BestMove.score) << srtString << " nodes " << (settings::parameter.HelperThreads + 1) * NodeCount
			<< " nps " << (settings::parameter.HelperThreads + 1) * NodeCount * 1000 / _thinkTime << " hashfull " << tt::GetHashFull()
			<< " tbhits " << effectiveTBHits
			<< " time " << _thinkTime
			<< " pv " << PrincipalVariation(npos, _depth) << sync_endl;
		else if (abs(static_cast<int>(BestMove.score)) <= static_cast<int>(VALUE_MATE)) {
			int pliesToMate = 0;
			if (static_cast<int>(BestMove.score) > 0) pliesToMate = VALUE_MATE - BestMove.score + 1; else pliesToMate = -BestMove.score - VALUE_MATE;
			sync_cout << "info depth " << _depth << " seldepth " << std::max(MaxDepth, _depth.load()) << " multipv " << pvIndx + 1 << " score mate " << pliesToMate / 2 << srtString << " nodes " << (settings::parameter.HelperThreads + 1) * NodeCount
				<< " nps " << (settings::parameter.HelperThreads + 1) * NodeCount * 1000 / _thinkTime << " hashfull " << tt::GetHashFull()
				<< " tbhits " << effectiveTBHits
				<< " time " << _thinkTime
				<< " pv " << PrincipalVariation(npos, _depth) << sync_endl;
		}
	}
}

void Search::debugInfo(std::string info)
{
	if (UciOutput && settings::parameter.Verbosity >= settings::VerbosityLevel::DEFAULT) sync_cout << "info string " << info << sync_endl;
}

void Search::log(std::string info)
{
	if (UciOutput && settings::parameter.Verbosity >= settings::VerbosityLevel::DEFAULT) sync_cout << "info string log " << info << sync_endl;
}

Move Search::GetBestBookMove(Position& pos, ValuatedMove* moves, int moveCount) {
	if (settings::options.getBool(settings::OPTION_OWN_BOOK) && !BookFile.empty()) {
		if (book == nullptr) book = std::make_unique<polyglot::Book>(BookFile);
		book->probe(pos, true, moves, moveCount);
	}
	return MOVE_NONE;
}

Search::Search() {
	for (int i = 0; i < 12; ++i)
		for (int j = 0; j < 64; ++j)
			counterMove.at(i).at(j) = MOVE_NONE;
	BestMove.move = MOVE_NONE;
	BestMove.score = VALUE_NOTYETDETERMINED;
	threadLocalData.cmHistory.initialize();
	threadLocalData.followupHistory.initialize();
	threadLocalData.History.initialize();
	_thinkTime = 0;
	PVMoves.fill(MOVE_NONE);
}

Search::~Search() {
	if (!BookFile.empty()) {
		BookFile = "";
	}
	if (XAnalysisOutput != nullptr) {
		delete XAnalysisOutput;
		XAnalysisOutput = nullptr;
	}
}

ValuatedMove Search::Think(Position& pos) {
	std::lock_guard<std::mutex> lgStart(mtxSearch);
	think_started.store(true);
	//slave threads
	std::vector<std::thread> subThreads;
	//Initialize Engine before starting the new search
	_thinkTime = 1; //to avoid divide by 0 errors
	ponderMove = MOVE_NONE;
	Value score = VALUE_ZERO;
#pragma warning(suppress: 26496)
	ValuatedMove lastBestMove = VALUATED_MOVE_NONE;
	int depth = 1;
	rootPosition = pos;
	rootPosition.ResetPliesFromRoot();
	settings::parameter.EngineSide = rootPosition.GetSideToMove();
	tt::newSearch();
	currmove_depth = settings::parameter.Verbosity == settings::VerbosityLevel::MAXIMUM ? 0 :
		settings::parameter.Verbosity == settings::VerbosityLevel::DEFAULT ? 5 : MAX_DEPTH + 1;
	//Get all root moves
	ValuatedMove* generatedMoves = rootPosition.GenerateMoves<LEGAL>();
	rootMoveCount = rootPosition.GeneratedMoveCount();
	if (rootMoveCount == 0) {
		BestMove.move = MOVE_NONE;
		BestMove.score = VALUE_ZERO;
		info(rootPosition, 0, SearchResultType::UNIQUE_MOVE);
		utils::debugInfo("No valid move!");
		return BestMove;
	}
	if (rootMoveCount == 1) {
		BestMove = *generatedMoves; //if there is only one legal move save time and return move immediately (although there is no score assigned)
		utils::debugInfo("Only one valid move!");
		goto END;
	}
	//check if book is available
	if (settings::options.getBool(settings::OPTION_OWN_BOOK) && !BookFile.empty()) {
		if (book == nullptr) book = std::make_unique<polyglot::Book>(BookFile);
		//Currently engine isn't looking for the best book move, but selects on of the available bookmoves by random, with entries weight used to define the
		//probability 
		const Move bookMove = book->probe(rootPosition, false, generatedMoves, rootMoveCount);
		if (bookMove != MOVE_NONE) {
			BestMove.move = bookMove;
			BestMove.score = VALUE_ZERO;
			utils::debugInfo("Book move");
			//Try to find a suitable move for pondering
			Position next(rootPosition);
			if (next.ApplyMove(bookMove)) {
				ValuatedMove* replies = next.GenerateMoves<LEGAL>();
				ponderMove = book->probe(next, true, replies, next.GeneratedMoveCount());
				if (ponderMove == MOVE_NONE && next.GeneratedMoveCount() > 0) ponderMove = replies->move;
			}
			info(rootPosition, 0, SearchResultType::BOOK_MOVE);
			goto END;
		}
	}
	//If a search move list is provided replace root moves by search moves
	if (searchMoves.size()) {
		rootMoveCount = gsl::narrow_cast<int>(searchMoves.size());
		for (int i = 0; i < rootMoveCount; ++i) rootMoves.at(i).move = searchMoves.at(i);
		searchMoves.clear();
	}
	else {
		memcpy(rootMoves.data(), generatedMoves, rootMoveCount * sizeof(ValuatedMove));
	}
	//Root probing of tablebases. This is done as suggested by SF: Keep only the winning, resp. drawing, moves in the move list
	//and then search normally. This way the engine will play "better" than by simply choosing the "best" tablebase move (which is
	//the move which minimizes the number until drawPlyCount is reset without changing the result
	tbHits = 0;
	probeTB = tablebases::MaxCardinality > 0;
	if (rootPosition.GetMaterialTableEntry()->IsTablebaseEntry()) {
		probeTB = false;
		tablebases::RootMoves tbMoves;
		for (int i = 0; i < rootMoveCount; ++i) tbMoves.emplace_back(rootMoves.at(i).move);
		if (tablebases::rank_root_moves(rootPosition, tbMoves)) {
			++tbHits;
			for (int i = 0; i < rootMoveCount; ++i) {
				rootMoves.at(i).move = tbMoves.at(i).pv.at(0);
				rootMoves.at(i).score = tbMoves.at(i).tbScore;
				BestMove = rootMoves.at(0); //if tablebase probe only returns one move => play it and done!
				info(rootPosition, 0, SearchResultType::TABLEBASE_MOVE);
				//utils::debugInfo("Tablebase move", toString(BestMove.move));
				goto END;
			}
		}
	}
	//SetRootMoveBoni();
	use_tt_in_qsearch = settings::parameter.USE_TT_IN_QSEARCH;
	Contempt = rootPosition.GetSideToMove() == Color::WHITE ? Eval(settings::parameter.Contempt, settings::parameter.Contempt / 2)
		: Eval(-settings::parameter.Contempt, -settings::parameter.Contempt / 2);
	//Initialize PV-Array
	PVMoves.fill(MOVE_NONE);
	//Special logic to get static evaluation via UCI: if go depth 0 is requested simply return static evaluation
	if (timeManager.GetMaxDepth() == 0) {
		BestMove.move = MOVE_NONE;
		BestMove.score = rootPosition.evaluate();
		if (abs(static_cast<int>(BestMove.score)) <= static_cast<int>(VALUE_MATE_THRESHOLD)) sync_cout << "info score cp " << static_cast<int>(BestMove.score) << sync_endl;
		else {
			int pliesToMate = 0;
			if (static_cast<int>(BestMove.score) > 0) pliesToMate = VALUE_MATE - BestMove.score; else pliesToMate = -BestMove.score - VALUE_MATE;
			sync_cout << "info score mate " << pliesToMate / 2 << sync_endl;
		}
		return BestMove;
	}
	for (auto m : rootMoves) {
		timeManager._moveNodeCounts[from(m.move)][to(m.move)] = 0;
	}
	Stop.store(false);
	if (settings::parameter.HelperThreads) {
		searchResults.clear();
		for (int i = 0; i <= settings::parameter.HelperThreads; ++i)
			searchResults.push_back(SearchResult{ VALUATED_MOVE_NONE, 0 });
		if (thread_pool == nullptr) thread_pool = new ThreadPool(settings::parameter.HelperThreads);
		else if (gsl::narrow_cast<int>(thread_pool->size()) != settings::parameter.HelperThreads) {
			delete thread_pool;
			thread_pool = new ThreadPool(settings::parameter.HelperThreads);
		}
		for (int i = 0; i < settings::parameter.HelperThreads; ++i)
			thread_pool->enqueue(std::bind(&Search::startHelper, this, std::placeholders::_1));
	}
	threadLocalData.id = 0;
	_depth.store(1);
	//Iterativ Deepening Loop
	while ((depth = _depth.load()) <= timeManager.GetMaxDepth()) {
		Value alpha = -VALUE_MATE, beta = VALUE_MATE, delta = gsl::narrow_cast<Value>(20);
		for (int pvIndx = 0; pvIndx < MultiPv && pvIndx < rootMoveCount; ++pvIndx) {
			if (depth >= 5 && MultiPv == 1 && std::abs(gsl::narrow_cast<int16_t>(score)) < VALUE_KNOWN_WIN) {
				//set aspiration window
				alpha = gsl::narrow_cast<Value>(std::max(score - delta, -VALUE_INFINITE));
				beta = std::min(static_cast<Value>(score + delta), VALUE_INFINITE);
			}
			else {
				alpha = -VALUE_MATE;
				beta = VALUE_MATE;
			}
			int depth2 = depth;
			while (true) {
				CHECK(rootPosition.GetPliesFromRoot() == 0)
					if (settings::parameter.HelperThreads > 0)
						score = SearchRoot<ThreadType::MASTER>(alpha, beta, rootPosition, depth2, rootMoves, PVMoves, threadLocalData, pvIndx);
					else
						score = SearchRoot<ThreadType::SINGLE>(alpha, beta, rootPosition, depth2, rootMoves, PVMoves, threadLocalData, pvIndx);
				CHECK(rootPosition.GetPliesFromRoot() == 0)
					//Best move is already in first place, this is assured by SearchRoot
					//therefore we sort only the other moves
					std::stable_sort(rootMoves.data() + pvIndx + 1, &rootMoves.at(rootMoveCount), sortByScore);
				if (Stopped()) {
					break;
				}
				if (score <= alpha) {
					//fail-low
					beta = (alpha + beta) / 2;
					alpha = gsl::narrow_cast<Value>(std::max(score - delta, -VALUE_INFINITE));
					if (settings::parameter.Verbosity >= settings::VerbosityLevel::LOW)
						info(rootPosition, pvIndx, SearchResultType::FAIL_LOW);
					//inform timemanager to assigne more time
					if (!PonderMode.load()) timeManager.reportFailLow();
				}
				else if (score >= beta && rootMoves.at(pvIndx).move == lastBestMove.move) {
					//Iteration completed
					BestMove = rootMoves.at(pvIndx);
					score = BestMove.score;
					break;
				}
				else if (score >= beta) {
					//fail-high
					alpha = (alpha + beta) / 2;
					beta = std::min(static_cast<Value>(score + delta), VALUE_INFINITE);
					depth2 = std::max(depth2 - 1, depth / 2);
					if (settings::parameter.Verbosity >= settings::VerbosityLevel::LOW)
						info(rootPosition, pvIndx, SearchResultType::FAIL_HIGH);
				}
				else {
					//Iteration completed
					BestMove = rootMoves.at(pvIndx);
					score = BestMove.score;
					break;
				}
				if (std::abs(gsl::narrow_cast<int16_t>(score)) >= VALUE_KNOWN_WIN) {
					alpha = -VALUE_MATE;
					beta = VALUE_MATE;
				}
				else delta += delta * 2;
			}
			const Time_t tNow = now();
			_thinkTime = std::max(tNow - timeManager.GetStartTime(), gsl::narrow_cast<int64_t>(1));
			if (!Stopped()) {
				//check if new deeper iteration shall be started
				if (!timeManager.ContinueSearch(depth, BestMove, NodeCount, tNow)) {
					Stop.store(true);
				}
			}
			else debugInfo("Iteration cancelled!");
			if (Stopped()) break;
			//send information to GUI
			if (settings::parameter.Verbosity >= settings::VerbosityLevel::LOW || MultiPv > 1)
				info(rootPosition, pvIndx);
			_depth.compare_exchange_strong(depth, depth + 1);
		}
		if (Stopped()) break;
		lastBestMove = BestMove;
		if (settings::parameter.HelperThreads) {
			searchResults.at(0).move = BestMove;
			searchResults.at(0).depth = _depth;
		}
	}
	Stop.store(true);
	while (thread_pool != nullptr && thread_pool->tasks_active()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
	if (settings::parameter.HelperThreads) {
		//Try to find best best move
		int bestDepth = searchResults.at(0).depth;
		for (auto m : searchResults) {
			if (m.move.move != MOVE_NONE && m.move.move != BestMove.move && m.depth > bestDepth) {
				BestMove = m.move;
				bestDepth = m.depth;
			}
		}
	}
END://when pondering engine must not return a best move before opponent moved => therefore let main thread wait	
	if (settings::parameter.Verbosity < settings::VerbosityLevel::LOW || MultiPv <= 1)
		info(rootPosition, 0);
	bool infoSent = false;
	while (PonderMode.load()) {
		if (!infoSent) utils::debugInfo("Waiting for opponent..");
		infoSent = true;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	//if (PVMoves.at(0) != MOVE_NONE && PVMoves.at(0) != BestMove.move) {
	//	std::stringstream ss;
	//	ss << "PV Move: " << toString(PVMoves.at(0)) << " Bestmove: " << toString(BestMove.move);
	//	utils::debugInfo(ss.str());
	//	BestMove.move = PVMoves.at(0);
	//}
	//If for some reason search did not find a best move return the  first one (to avoid loss and it's anyway the best guess then)
	if (BestMove.move == MOVE_NONE) BestMove = rootMoves.at(0);
	return BestMove;
}

//slave thread
void Search::startHelper(int id) {
#ifdef _DEBUG
	sync_cout << "Helper task " << id << " started" << sync_endl;
#endif // _DEBUG
	WinProcGroup::bindThisThread(id);
	int depth = 1;
	std::array<Move, PV_MAX_LENGTH> PVMovesLocal;
	PVMovesLocal.fill(MOVE_NONE);
#pragma warning(suppress: 26496)
	ValuatedMove lastBestMove = VALUATED_MOVE_NONE;
	std::array<ValuatedMove, MAX_MOVE_COUNT> moves;
	moves = rootMoves;
	ThreadData h;
	h.id = id;
	//Iterative Deepening Loop
	Value score = VALUE_ZERO;
	while (!Stop.load() && depth < MAX_DEPTH) {
		depth = _depth.load();
		Value alpha = -VALUE_MATE, beta = VALUE_MATE, delta = gsl::narrow_cast<Value>(20);
		if (depth >= 5) {
			//set aspiration window
			alpha = gsl::narrow_cast<Value>(std::max(score - delta, -VALUE_INFINITE));
			beta = std::min(static_cast<Value>(score + delta), VALUE_INFINITE);
		}
		while (true && !Stop.load()) {
			CHECK(rootPosition.GetPliesFromRoot() == 0)
				score = SearchRoot<ThreadType::SLAVE>(alpha, beta, rootPosition, depth, moves, PVMovesLocal, h);
			CHECK(rootPosition.GetPliesFromRoot() == 0)
				if (score <= alpha) {
					//fail-low
					beta = (alpha + beta) / 2;
					alpha = gsl::narrow_cast<Value>(std::max(score - delta, -VALUE_INFINITE));
				}
				else if (score >= beta && moves.at(0).move == lastBestMove.move) {
					//Iteration completed
					score = moves.at(0).score;
					break;
				}
				else if (score >= beta) {
					//fail-high
					alpha = (alpha + beta) / 2;
					beta = std::min(static_cast<Value>(score + delta), VALUE_INFINITE);
				}
				else {
					//Iteration completed
					score = moves.at(0).score;
					break;
				}
			if (std::abs(gsl::narrow_cast<int16_t>(score)) >= VALUE_KNOWN_WIN) {
				alpha = -VALUE_MATE;
				beta = VALUE_MATE;
			}
			else delta += delta * 2;
		}
		lastBestMove = moves.at(0);
		if (!Stop.load()) {
			searchResults.at(id).move = lastBestMove;
			searchResults.at(id).depth = depth;
			_depth.compare_exchange_strong(depth, depth + 1);
		}
	}
#ifdef _DEBUG
	sync_cout << "Helper task " << id << " done" << sync_endl;
#endif // _DEBUG
}

bool Search::isQuiet(Position& pos) {
	const Value evaluationDiff = pos.GetStaticEval() - QSearch<ThreadType::SINGLE>(-VALUE_MATE, VALUE_MATE, pos, 0, threadLocalData);
	return std::abs(gsl::narrow_cast<int16_t>(evaluationDiff)) <= 30;
}

Value Search::qscore(Position* pos)
{
	return QSearch<ThreadType::SINGLE>(-VALUE_MATE, VALUE_MATE, *pos, 0, threadLocalData);
}

void Search::updateCutoffStats(ThreadData& tlData, const Move cutoffMove, int depth, Position& pos, int moveIndex) noexcept {
	if (moveIndex == -1 || pos.IsQuiet(cutoffMove)) {
		Piece movingPiece = pos.GetPieceOnSquare(from(cutoffMove));
		Square toSquare = to(cutoffMove);
		if (moveIndex >= 0) {
			tlData.killerManager.store(pos, cutoffMove);
		}
		const Value v = gsl::narrow_cast<Value>(depth * depth);
		tlData.History.update(gsl::narrow_cast<Value>(-depth * tlData.History.getValue(movingPiece, cutoffMove) / 64), movingPiece, cutoffMove);
		tlData.History.update(v, movingPiece, cutoffMove);
		Piece prevPiece = BLANK;
		Square prevTo = OUTSIDE;
		Piece prev2Piece = BLANK;
		Square prev2To = OUTSIDE;
		//Piece ownPrevPiece = BLANK;
		//Square ownPrevTo = OUTSIDE;
		Move lastApplied;
		if ((lastApplied = FixCastlingMove(pos.GetLastAppliedMove())) != MOVE_NONE) {
			prevTo = to(lastApplied);
			prevPiece = pos.GetPieceOnSquare(prevTo);
			counterMove.at(static_cast<int>(pos.GetPieceOnSquare(prevTo))).at(prevTo) = cutoffMove;
			tlData.cmHistory.update(gsl::narrow_cast<Value>(-depth * tlData.cmHistory.getValue(prevPiece, prevTo, movingPiece, toSquare) / 64), prevPiece, prevTo, movingPiece, toSquare);
			tlData.cmHistory.update(v, prevPiece, prevTo, movingPiece, toSquare);
			Move lastApplied2 = MOVE_NONE;
			if (pos.Previous() && (lastApplied2 = FixCastlingMove(pos.Previous()->GetLastAppliedMove())) != MOVE_NONE) {
				prev2To = to(lastApplied2);
				prev2Piece = pos.Previous()->GetPieceOnSquare(prev2To);
				tlData.followupHistory.update(gsl::narrow_cast<Value>(-depth * tlData.followupHistory.getValue(prev2Piece, prev2To, movingPiece, toSquare) / 64), prev2Piece, prev2To, movingPiece, toSquare);
				tlData.followupHistory.update(v, prev2Piece, prev2To, movingPiece, toSquare);
			}
		}
		if (moveIndex > 0) {
			int moveCount;
			ValuatedMove* alreadyProcessedQuiets = pos.GetMoves(moveCount);
			moveCount = std::min(moveIndex, moveCount);
			for (int i = 0; i < moveCount; ++i) {
				if (alreadyProcessedQuiets->move != cutoffMove && pos.IsQuiet(alreadyProcessedQuiets->move)) {
					const Move alreadyProcessedMove = FixCastlingMove(alreadyProcessedQuiets->move);
					movingPiece = pos.GetPieceOnSquare(from(alreadyProcessedMove));
					toSquare = to(alreadyProcessedMove);
					tlData.History.update(gsl::narrow_cast<Value>(-depth * tlData.History.getValue(movingPiece, alreadyProcessedMove) / 64), movingPiece, alreadyProcessedMove);
					tlData.History.update(-v, movingPiece, alreadyProcessedMove);
					if (pos.GetLastAppliedMove() != MOVE_NONE) {
						tlData.cmHistory.update(gsl::narrow_cast<Value>(-depth * tlData.cmHistory.getValue(prevPiece, prevTo, movingPiece, toSquare) / 64), prevPiece, prevTo, movingPiece, toSquare);
						tlData.cmHistory.update(-v, prevPiece, prevTo, movingPiece, toSquare);
					}
					if (prev2To != Square::OUTSIDE) {
						tlData.followupHistory.update(gsl::narrow_cast<Value>(-depth * tlData.followupHistory.getValue(prev2Piece, prev2To, movingPiece, toSquare) / 64), prev2Piece, prev2To, movingPiece, toSquare);
						tlData.followupHistory.update(-v, prev2Piece, prev2To, movingPiece, toSquare);
					}
				}
				//if (ownPrevTo != OUTSIDE) {
				//	cmHistory.update(-v, ownPrevPiece, ownPrevTo, movingPiece, toSquare);
				//}
				alreadyProcessedQuiets++;
			}
		}
	}
}

ThreadPool::ThreadPool(size_t numberOfThreads)
{
	start(numberOfThreads);
}

ThreadPool::~ThreadPool()
{
	stop();
}

void ThreadPool::enqueue(Task t)
{
	{
		std::unique_lock<std::mutex> lock(mtxStartTask);
		tasks.emplace(t);
	}
	cvStartTask.notify_one();
}

void ThreadPool::start(size_t numberOfThreads)
{
	for (auto i = 0u; i < numberOfThreads; ++i) {
		threads.emplace_back([=] {
			while (true) {
				Task task;
				{
					std::unique_lock<std::mutex> lock(mtxStartTask);
					cvStartTask.wait(lock, [=] { return shutdown || !tasks.empty(); });

					if (shutdown) break;
					task = std::move(tasks.front());
					tasks.pop();
				}
				active.fetch_add(1);
				task(i + 1);
				active.fetch_sub(1);
			}
		});
	}
}

void ThreadPool::stop()
{
	{
		std::unique_lock<std::mutex> lock(mtxStartTask);
		shutdown = true;
	}
	cvStartTask.notify_all();

	for (auto& th : threads) th.join();
	threads.clear();
}
