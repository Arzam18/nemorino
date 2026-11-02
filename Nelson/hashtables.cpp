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

#include "hashtables.h"
#include "position.h"
#include <iostream>
#ifndef _M_ARM
#ifdef _MSC_VER
#include <mmintrin.h>
#endif
#endif
#include "stdlib.h"
#include "search.h"

namespace pawn {

	Entry Table[PAWN_TABLE_SIZE];

	void initialize() noexcept {
		clear();
	}

	void clear() noexcept {
		std::fill_n(Table, PAWN_TABLE_SIZE, Entry());
	}

#pragma warning( push )
#pragma warning( disable : 26446 )
	Entry* probe(const Position& pos) noexcept {
		Entry* result = &Table[pos.GetPawnKey() & (PAWN_TABLE_SIZE - 1)];
		if (result->Key == pos.GetPawnKey()) return result;
		result->Score = Eval(0);
		const Bitboard bbWhite = pos.PieceBB(PAWN, Color::WHITE);
		const Bitboard bbBlack = pos.PieceBB(PAWN, Color::BLACK);
		const Bitboard bbFilesWhite = FileFill(bbWhite);
		const Bitboard bbFilesBlack = FileFill(bbBlack);
		const Bitboard attacksWhite = ((bbWhite << 9) & NOT_A_FILE) | ((bbWhite << 7) & NOT_H_FILE);
		const Bitboard attacksBlack = ((bbBlack >> 9) & NOT_H_FILE) | ((bbBlack >> 7) & NOT_A_FILE);
		//frontspans
		const Bitboard bbWFrontspan = FrontFillNorth(bbWhite);
		const Bitboard bbBFrontspan = FrontFillSouth(bbBlack);
		//attacksets
		const Bitboard bbWAttackset = FrontFillNorth(attacksWhite);
		const Bitboard bbBAttackset = FrontFillSouth(attacksBlack);
		Bitboard ppW = bbWhite & (~(bbBAttackset | bbBFrontspan));
		Bitboard ppB = bbBlack & (~(bbWAttackset | bbWFrontspan));
		result->passedPawns = ppW | ppB;
		result->openFiles = Fileset(~bbFilesWhite & ~bbFilesBlack);
		result->halfOpenFiles[static_cast<int>(Color::WHITE)] = Fileset(~bbFilesWhite & bbFilesBlack);
		result->halfOpenFiles[static_cast<int>(Color::BLACK)] = Fileset(bbFilesWhite & ~bbFilesBlack);
		while (ppW) {
			const int rank = (lsb(ppW) >> 3) - 1;
			result->Score += settings::parameter.PASSED_PAWN_BONUS.at(rank);
			if (rank > 0 && (isolateLSB(ppW) & attacksWhite) != 0) result->Score += settings::parameter.BONUS_PROTECTED_PASSED_PAWN.at(rank);
			ppW &= ppW - 1;
		}
		while (ppB) {
			const int rank = 6 - (lsb(ppB) >> 3);
			result->Score -= settings::parameter.PASSED_PAWN_BONUS.at(rank);
			if (rank > 0 && (isolateLSB(ppB) & attacksBlack) != 0) result->Score -= settings::parameter.BONUS_PROTECTED_PASSED_PAWN.at(rank);
			ppB &= ppB - 1;
		}
		//Candidate passed pawns
		Bitboard potentialCandidates = bbWhite & ~bbBFrontspan & bbBAttackset & ~attacksBlack; //open, but not passed pawns
		while (potentialCandidates) {
			const Bitboard candidateBB = isolateLSB(potentialCandidates);
			const Bitboard sentries = FrontFillNorth(((candidateBB << 17) & NOT_A_FILE) | ((candidateBB << 15) & NOT_H_FILE)) & bbBlack;
			const Bitboard helper = FrontFillSouth(sentries >> 16) & bbWhite;
			//Should be replaced with 
			//Bitboard helper = FrontFillSouth(sentries >> 16) & bbWhite & ~(bbBlack >> 8);
			//To exclude blocked helpers
			if (popcount(helper) >= popcount(sentries)) {
				result->Score += settings::parameter.BONUS_CANDIDATE * (lsb(candidateBB) >> 3);
			}
			potentialCandidates &= potentialCandidates - 1;
		}
		potentialCandidates = bbBlack & ~bbWFrontspan & bbWAttackset & ~attacksWhite; //open, but not passed pawns
		while (potentialCandidates) {
			const Bitboard candidateBB = isolateLSB(potentialCandidates);
			const Bitboard sentries = FrontFillSouth(((candidateBB >> 15) & NOT_A_FILE) | ((candidateBB >> 17) & NOT_H_FILE)) & bbWhite;
			const Bitboard helper = FrontFillNorth(sentries << 16) & bbBlack;

			if (popcount(helper) >= popcount(sentries)) {
				result->Score -= settings::parameter.BONUS_CANDIDATE * (7 - (lsb(candidateBB) >> 3));
			}
			potentialCandidates &= potentialCandidates - 1;
		}
		//Levers 
		Bitboard levers = bbWhite & (RANK5 | RANK6) & attacksBlack;
		while (levers) {
			const int leverRank = gsl::narrow_cast<int>(lsb(levers) >> 3) - 3;
			assert(leverRank == 1 || leverRank == 2);
			result->Score += leverRank * settings::parameter.BONUS_LEVER;
			levers &= levers - 1;
		}
		levers = bbBlack & (RANK4 | RANK3) & attacksWhite;
		while (levers) {
			const int leverRank = 4 - gsl::narrow_cast<int>(lsb(levers) >> 3);
			assert(leverRank == 1 || leverRank == 2);
			result->Score -= leverRank * settings::parameter.BONUS_LEVER;
			levers &= levers - 1;
		}
		//pawn islands
		result->Score += settings::parameter.MALUS_ISLAND_COUNT * (popcount(IslandsEastFiles(bbBlack)) - popcount(IslandsEastFiles(bbWhite)));
		//backward pawns
		const Bitboard bbWeak = GetWeakPawns(bbWhite, bbBlack, attacksWhite, attacksBlack);
		result->Score += settings::parameter.MALUS_WEAK_PAWN * (popcount(bbWeak & bbBlack) - popcount(bbWeak & bbWhite));
		const Bitboard bbWBackward = bbWeak & bbWhite & ~result->passedPawns & FileFill(result->halfOpenFiles[static_cast<int>(Color::BLACK)]) & bbBAttackset & ~attacksBlack;
		const Bitboard bbBBackward = bbWeak & bbBlack & ~result->passedPawns & FileFill(result->halfOpenFiles[static_cast<int>(Color::WHITE)]) & bbWAttackset & ~attacksWhite;
		result->Score += settings::parameter.MALUS_BACKWARD_PAWN * (popcount(bbBBackward) - popcount(bbWBackward));
		//doubled pawns
		Bitboard doubled = FrontFillNorth(bbWhite << 8) & bbWhite;
		result->Score -= popcount(doubled) * settings::parameter.MALUS_DOUBLED_PAWN;
		doubled = FrontFillSouth(bbBlack >> 8) & bbBlack;
		result->Score += popcount(doubled) * settings::parameter.MALUS_DOUBLED_PAWN;
		result->Key = pos.GetPawnKey();
		return result;
	}
}

namespace tt {

	int initializedSizeInMB = 0;

	uint8_t _generation = 0;
	uint64_t ProbeCounter = 0;
	uint64_t HitCounter = 0;
	uint64_t FillCounter = 0;

	uint64_t GetProbeCounter() noexcept { return ProbeCounter; }
	uint64_t GetHitCounter() noexcept { return HitCounter; }
	uint64_t GetFillCounter() noexcept { return FillCounter; }

	std::unique_ptr<Cluster[]> Table;
	uint64_t MASK;

	uint64_t GetHashFull() noexcept {
		if (settings::parameter.HelperThreads == 0)
			return 1000 * FillCounter / GetEntryCount();
		else {
			uint64_t result = 0;
			for (int i = 0; i < 250; ++i) {
				for (int j = 0; j < CLUSTER_SIZE; ++j) {
					if (Table[i].entry[j].key) ++result;
				}

			}
			return result;
		}
	}

	//Calculates the number of clusters in the transposition table if the table size should use
	//sizeMB Megabytes (which is treated as upper limit)
	uint64_t CalculateClusterCount(int SizeMB) noexcept {
		uint64_t clusterCount = SizeMB * 1024ull * 1024 / sizeof(Cluster);
		clusterCount = 1ull << msb(clusterCount);
		if (clusterCount < 1024) clusterCount = 1024;
		return clusterCount;
	}

	void InitializeTranspositionTable() {
		const int newHashSize = settings::options.getInt(settings::OPTION_HASH);
		if (initializedSizeInMB != newHashSize) {
			const uint64_t clusterCount = CalculateClusterCount(newHashSize);
			Table = std::make_unique<Cluster[]>(clusterCount);
			clear();
			MASK = clusterCount - 1;
			ResetCounter();
			initializedSizeInMB = newHashSize;
		}
	}

	void clear() noexcept {
		std::memset(Table.get(), 0, (MASK + 1) * sizeof(Cluster));
	}

	void Clear() noexcept {

	}

	uint64_t NMASK;

	Entry* firstEntry(const uint64_t hash) noexcept {
		return &Table[hash & MASK].entry[0];
	}

	void prefetch(uint64_t hash) noexcept {
#ifdef _MSC_VER
		_mm_prefetch(reinterpret_cast<char*>(&Table[hash & MASK]), _MM_HINT_T0);
#endif // _MSC_VER
#ifdef __GNUC__
		__builtin_prefetch(reinterpret_cast<char*>(&Table[hash & MASK]));
#endif // __GNUC__
	}

	uint64_t GetClusterCount() noexcept {
		return MASK + 1;
	}
	uint64_t GetEntryCount() noexcept {
		return GetClusterCount() * CLUSTER_SIZE;
	}


	bool dumpTT(std::ostream& stream) {
		for (int i = 0; i <= MASK; ++i) {
			const Cluster* cluster = &Table[i];
			for (int j = 0; j < CLUSTER_SIZE; ++j) {
				if (cluster->entry[j].generation() == _generation) {
					stream.write(reinterpret_cast<const char*>(&cluster->entry[j]), sizeof(Entry));
				}
			}
		}
		return true;
	}
}
#pragma warning( pop )

namespace killer {

	Move Manager::getMove(const Position& pos, int index) const noexcept
	{
		const size_t indx = getIndex(pos) + index;
		return indx < PLY_TABLE_SIZE ? plyTable.at(getIndex(pos) + index) : MOVE_NONE;
	}


	void Manager::store(const Position& pos, Move move) noexcept
	{
		const size_t indx = getIndex(pos);
		if (indx < PLY_TABLE_SIZE - 1 && plyTable.at(indx) != move) {
			plyTable.at(indx + 1) = plyTable.at(indx);
			plyTable.at(indx) = move;
		}
	}

	void Manager::clear() noexcept
	{
		std::memset(plyTable.data(), 0, PLY_TABLE_SIZE * sizeof(Move));
	}

	int Manager::getIndex(const Position& pos) const noexcept
	{
		return NB_SLOTS_KILLER * (2 * pos.GetPliesFromRoot() + static_cast<int>(pos.GetSideToMove()));
	}

	int Manager::getIndex2(const Position& pos) const noexcept
	{
		return NB_SLOTS_KILLER * (2 * (pos.GetPliesFromRoot() + 1) + static_cast<int>(pos.GetSideToMove()) ^ 1);
	}

	bool Manager::isKiller(const Position& pos, Move move) const noexcept
	{
		const size_t index = getIndex(pos);
		return (index < PLY_TABLE_SIZE - 1) && ((move == plyTable.at(index)) || (move == plyTable.at(index + 1)));
	}

	void Manager::enterLevel(const Position& pos) noexcept
	{
		const size_t index = getIndex2(pos) + 2 * NB_SLOTS_KILLER;
		if ((index >= PLY_TABLE_SIZE - 1) || ((index - 4 * NB_SLOTS_KILLER) < 0))
			return;
		if (pos.Checked()) {
			plyTable.at(index) = plyTable.at(index - 4 * NB_SLOTS_KILLER);
			plyTable.at(index + 1) = plyTable.at(index - 4 * NB_SLOTS_KILLER + 1);
		}
		else {
			plyTable.at(index) = MOVE_NONE;
			plyTable.at(index + 1) = MOVE_NONE;
		}
	}

}