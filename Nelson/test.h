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

#pragma once
#include <vector>
#include "types.h"
#include "board.h"
#include "position.h"

namespace test {

	enum PerftType {
		BASIC,  //All moves are generated together
		P1, //tactical and Quiet Moves are generated seperately
		P2, //Winning, Equal, Loosing Captures and Quiets are generated separately
		P3,  //Move iterator is used
		P4   //Legal move generation
	};

	int64_t benchmark(int depth);
	int64_t benchmark(std::string filename, int depth);
	int64_t bench(std::vector<std::string> fens, int depth, int64_t &totalTime);
	int64_t bench(int depth, int64_t &totalTime); //Benchmark positions from SF
	int64_t bench2(int depth, int64_t &totalTime); //100 Random positions from GM games
	int64_t bench3(int depth, int64_t &totalTime); //200 Random positions from own games
	int64_t bench(std::string filename, int depth, int64_t &totalTime);


	uint64_t perft(Position &pos, int depth);
	uint64_t perft3(Position &pos, int depth);
	uint64_t perft4(Position &pos, int depth);
	uint64_t perftcomb(Position &pos, int depth);

	void divide(Position &pos, int depth);
	void divide3(Position &pos, int depth);
	bool testPerft(PerftType perftType = BASIC);
	void testPolyglotKey();
	bool testSEE();
	std::vector<std::string> readTextFile(std::string file);
	void testCheckQuietCheckMoveGeneration();
	void testTacticalMoveGeneration();
	void testSearch(Position &pos, int depth);
	void testFindMate();
	void testResult();
	void testRepetition();
	void testKPK();
	bool testBBOperations();
	bool testPopcount();
	bool testLSB();
	bool testMSB();
	bool testGivesCheck();

	bool testVerticalSymmetry();

	bool testingPlatformSpecifics();

	bool testTB();

	bool testPack();

	uint64_t nnue_evaluate(std::string ifile, std::string ofile, std::string eval_file);

}
