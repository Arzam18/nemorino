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

#include <stdio.h>
#include <iostream>
#include <string> 
#include <chrono>
#include <iomanip>
#include <fstream>
#include <string>
#include <map>
#include <cstdlib>
#include <vector>
#include "test.h"
#include "search.h"
#include "hashtables.h"
#include "timemanager.h"

namespace test {

	int64_t benchmark(int depth) {
		std::cout << "Running bench at depth " << depth << " hash " << settings::options.getInt(settings::OPTION_HASH) << " threads " << settings::parameter.HelperThreads + 1 << std::endl;
		int64_t runtime = 0;
		int64_t totalTime = 0;
		int64_t totalNodes = bench(depth, runtime);
		totalTime += runtime;
		totalNodes += bench2(depth, runtime);
		totalTime += runtime;
		std::cerr << "\n==========================="
			<< "\nTotal time (ms) : " << totalTime
			<< "\nNodes searched  : " << totalNodes
			<< "\nNodes/second    : " << 1000 * totalNodes / totalTime << std::endl;

		return totalNodes;
	}


	int64_t benchmark(std::string filename, int depth) {
		std::string line;
		int64_t runtime = 0;
		std::ifstream text(filename);
		if (text.is_open())
		{
			std::vector<std::string> fens;
			while (getline(text, line)) fens.push_back(line);
			text.close();
			const int64_t totalNodes = bench(fens, depth, runtime);
			std::cerr << "\n==========================="
				<< "\nTotal time (ms) : " << runtime
				<< "\nNodes searched  : " << totalNodes
				<< "\nNodes/second    : " << 1000 * totalNodes / runtime << std::endl;
			return totalNodes;
		}
		else return -1;
	}

	int64_t bench(std::string filename, int depth, int64_t& totalTime) {
		std::string line;
		std::ifstream text(filename);
		if (text.is_open())
		{
			std::vector<std::string> fens;
			while (getline(text, line)) fens.push_back(line);
			text.close();
			return bench(fens, depth, totalTime);
		}
		else return -1;
	}

	std::vector<std::string> benchFens1() {
		std::vector<std::string> fens = {
			"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
			"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 10",
			"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 11",
			"4rrk1/pp1n3p/3q2pQ/2p1pb2/2PP4/2P3N1/P2B2PP/4RRK1 b - - 7 19",
			"rq3rk1/ppp2ppp/1bnpb3/3N2B1/3NP3/7P/PPPQ1PP1/2KR3R w - - 7 14",
			"r1bq1r1k/1pp1n1pp/1p1p4/4p2Q/4Pp2/1BNP4/PPP2PPP/3R1RK1 w - - 2 14",
			"r3r1k1/2p2ppp/p1p1bn2/8/1q2P3/2NPQN2/PPP3PP/R4RK1 b - - 2 15",
			"r1bbk1nr/pp3p1p/2n5/1N4p1/2Np1B2/8/PPP2PPP/2KR1B1R w kq - 0 13",
			"r1bq1rk1/ppp1nppp/4n3/3p3Q/3P4/1BP1B3/PP1N2PP/R4RK1 w - - 1 16",
			"4r1k1/r1q2ppp/ppp2n2/4P3/5Rb1/1N1BQ3/PPP3PP/R5K1 w - - 1 17",
			"2rqkb1r/ppp2p2/2npb1p1/1N1Nn2p/2P1PP2/8/PP2B1PP/R1BQK2R b KQ - 0 11",
			"r1bq1r1k/b1p1npp1/p2p3p/1p6/3PP3/1B2NN2/PP3PPP/R2Q1RK1 w - - 1 16",
			"3r1rk1/p5pp/bpp1pp2/8/q1PP1P2/b3P3/P2NQRPP/1R2B1K1 b - - 6 22",
			"r1q2rk1/2p1bppp/2Pp4/p6b/Q1PNp3/4B3/PP1R1PPP/2K4R w - - 2 18",
			"4k2r/1pb2ppp/1p2p3/1R1p4/3P4/2r1PN2/P4PPP/1R4K1 b - - 3 22",
			"3q2k1/pb3p1p/4pbp1/2r5/PpN2N2/1P2P2P/5PP1/Q2R2K1 b - - 4 26",
			"6k1/6p1/6Pp/ppp5/3pn2P/1P3K2/1PP2P2/3N4 b - - 0 1",
			"3b4/5kp1/1p1p1p1p/pP1PpP1P/P1P1P3/3KN3/8/8 w - - 0 1",
			"2K5/p7/7P/5pR1/8/5k2/r7/8 w - - 0 1",
			"8/6pk/1p6/8/PP3p1p/5P2/4KP1q/3Q4 w - - 0 1",
			"7k/3p2pp/4q3/8/4Q3/5Kp1/P6b/8 w - - 0 1",
			"8/2p5/8/2kPKp1p/2p4P/2P5/3P4/8 w - - 0 1",
			"8/1p3pp1/7p/5P1P/2k3P1/8/2K2P2/8 w - - 0 1",
			"8/pp2r1k1/2p1p3/3pP2p/1P1P1P1P/P5KR/8/8 w - - 0 1",
			"8/3p4/p1bk3p/Pp6/1Kp1PpPp/2P2P1P/2P5/5B2 b - - 0 1",
			"5k2/7R/4P2p/5K2/p1r2P1p/8/8/8 b - - 0 1",
			"6k1/6p1/P6p/r1N5/5p2/7P/1b3PP1/4R1K1 w - - 0 1",
			"1r3k2/4q3/2Pp3b/3Bp3/2Q2p2/1p1P2P1/1P2KP2/3N4 w - - 0 1",
			"6k1/4pp1p/3p2p1/P1pPb3/R7/1r2P1PP/3B1P2/6K1 w - - 0 1",
			"8/3p3B/5p2/5P2/p7/PP5b/k7/6K1 w - - 0 1"
		};
		return fens;
	}

	std::vector<std::string> benchFens2() {
		std::vector<std::string> fens = {
		"5r2/4r1kp/1pnb2p1/p4p2/1P1P4/1NPBpRPP/1P6/5RK1 w - a6",
		"1Q6/8/2N2k2/8/2p4p/6bK/1P6/5n2 b - -",
		"8/1B2k3/8/1p5R/5n1p/PKP2N1r/1P6/8 b - -",
		"r3kb1r/pp5p/q3p1p1/2p1Np2/1n2P2P/3PBQ2/P4PP1/1R3K1R w - f6",
		"8/2R2p2/4pk2/2pr3p/8/5K1P/5PP1/8 w - -",
		"8/5p2/b5p1/3p4/3k4/1R3P1P/6P1/6K1 w - -",
		"rnbqk2r/pp2ppbp/2p2np1/3p4/2PP4/1QN3P1/PP2PPBP/R1B1K1NR b KQkq -",
		"2b2r2/5k1p/1p6/p1pP1pp1/B1P5/1P6/P5PP/4R1K1 w - g6",
		"r1bq1rk1/2p2pp1/p1n2n1p/P1b1p3/1p2P3/1B3N2/1PPN1PPP/R1BQR1K1 w - -",
		"r1bqr1k1/1pp2pp1/1bnp1n1p/p3p3/2P5/P1NP2PP/1PN1PPB1/1RBQ1RK1 b - -",
		"rnbqkbnr/pp2pppp/8/2pp4/4P3/2P5/PP1P1PPP/RNBQKBNR w KQkq d6",
		"4Q1k1/p4p2/1qp1p1p1/2p4p/2P4P/2P1P1P1/P4PK1/8 b - -",
		"4k2r/pR1n1Np1/1n6/1rBp4/3Pp3/4P3/P2K1P1P/6R1 b k -",
		"1k1r3r/p1q1np1P/1p2p3/2n1P3/3p1PP1/P1p4R/2P1N1Q1/1RB1K3 b - -",
		"1rbqr1k1/p2nbppp/2p5/3p4/5B2/1PNB1Q1P/P1P2PP1/3R1RK1 b - -",
		"r1bq1rk1/5pbp/p2ppnp1/1p6/3BPP2/1BN2Q2/PPP3PP/R3K2R w KQ b6",
		"r7/5k1p/6p1/2R1B3/1P2P3/4K3/6bP/8 w - -",
		"rnbq1rk1/p4ppp/1p2pn2/6B1/1bBP4/2N2N2/PP3PPP/R2Q1RK1 b - -",
		"5rk1/p1r1b1pp/bp2Nn2/8/1P6/P7/1B3PPP/R3R1K1 b - -",
		"8/4bk2/p7/1p4P1/1PbB2n1/P5Pp/7P/4R1K1 b - -",
		"q2rr1k1/pb2bppp/1p2p1n1/8/2PP2n1/PQ3NPP/1B1N1P2/3RRBK1 b - -",
		"7k/p1r3p1/1p2P1Qp/5P2/8/P3q3/5RK1/8 b - -",
		"r1bq1rk1/2p1npbp/p2p2p1/np6/3PP3/2N2N1P/PPB2PP1/R1BQ1RK1 b - -",
		"2r3k1/5p2/6p1/R6p/1p3B2/1qb1QP1P/6PK/8 b - -",
		"2rqr1k1/1pB2pp1/p1n3b1/8/1P1pP1P1/P2B1N2/3Q1PK1/3RR3 b - -",
		"r3r1k1/pbqn1pbp/1pp2np1/2P5/3pP3/2N1BN1P/P1Q2PP1/3RRBK1 w - -",
		"2r2rk1/1p3pp1/p6p/q2n4/2n1N3/3Q1PP1/PP1B1R1P/R4K2 b - -",
		"2k4r/2qr1p2/p2p4/2p4p/P1N1P1p1/1Q6/2P2PPb/1R2R2K b - -",
		"5n1k/p6p/5N1q/2Pp4/8/PP1r2b1/1B4P1/RQ5K w - -",
		"r2qkb1r/pp1n1ppp/2p5/2p1p2b/4P3/3P1N1P/PPP2PP1/R1BQKN1R w KQkq e6",
		"r1b2rk1/3nbppp/nqp1p3/p1N5/1p1PN3/1P2Q1P1/1B2PPBP/R4RK1 w - -",
		"6k1/5bp1/8/2K1BP2/5P2/8/1R6/6r1 b - -",
		"6k1/6p1/8/2K1BP1b/5P2/8/1R6/6r1 w - -",
		"rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6",
		"rnbqk2r/1p2bppp/p2p1n2/4p3/4P3/1NN5/PPP1BPPP/R1BQ1RK1 b kq -",
		"r5k1/5ppp/pr2pn2/8/2P5/5NP1/P4PP1/1R3RK1 b - -",
		"rn2k2r/ppp3pp/3n4/4q3/4P3/P2B1P2/1PQ4P/R1B1K2R b KQkq -",
		"6k1/2p5/1p1p2q1/p2P1b1p/2P1p2B/KP3rP1/P7/2Q1R3 w - -",
		"2b2rk1/Q1R2ppp/4p3/8/4q3/1B4P1/P4PKP/8 w - -",
		"r1bq1rk1/pp2ppbp/2n2np1/2pp4/8/P1NP1NP1/1PP1PPBP/R1BQ1RK1 w - d6",
		"3kr3/1p1r4/p1p2p2/P1P3p1/1P2PbP1/7P/2K1R3/4NB2 w - -",
		"8/6k1/6p1/R4p2/Pp5p/5B2/1Pq5/3NK3 b - -",
		"1r4k1/3qnppp/1r6/pp1pP3/2pP1P2/P1P2B2/1R1Q2PP/1R5K w - -",
		"3rr1k1/1p3p2/1pp3p1/8/2q1P1pP/P1R3P1/1PQ2P2/4R1K1 b - -",
		"r1bq1rk1/pp2bppp/2n1p3/2pn4/3P4/2NBPN2/PP3PPP/R1BQ1RK1 w - -",
		"2k5/8/p2pN1p1/3P2P1/1P6/3K3n/8/8 w - -",
		"rnbqkb1r/p2p1ppp/1p2p3/4P3/3P4/2P2N2/P4PPP/R1BQKB1R b KQkq -",
		"r1bqr1k1/1pp2pb1/p1n2npp/3pp3/4P3/2PP1B1P/PPQ2PPN/R1B1RNK1 b - -",
		"r1rq2k1/1b1nbppp/3p1n2/p2Pp3/PpN1P3/3BBN2/1P2QPPP/2R2RK1 w - -",
		"8/5ppk/5n1p/5Q2/2q3P1/7P/5BK1/8 b - -",
		"rnbqnrk1/2p1ppbp/pp1p2p1/4P3/P2P4/2N2N2/1PP1BPPP/R1BQR1K1 b - -",
		"r1bqnrk1/2p1ppbp/ppnp2p1/4P3/P2P4/2N2N2/1PP1BPPP/R1BQR1K1 w - -",
		"8/5k2/8/8/5PK1/6p1/8/8 w - -",
		"8/2q3k1/1ppbb1pp/2p1p3/2P1P3/P5P1/1P3PN1/3Q1BK1 b - -",
		"r3qrk1/p7/2p1b3/5pp1/1nNPp3/4P3/P1Q2BPP/R4RK1 w - -",
		"7r/2R4P/p7/Pp6/8/1k6/3p2K1/8 w - -",
		"8/pp3n1p/2p1kp1P/2P1p1p1/1P2P3/3KNPP1/P7/8 w - -",
		"8/pp3n1p/2p1kp1P/2P1pNp1/1P2P3/3K1PP1/P7/8 b - -",
		"8/6k1/8/1R6/5PKP/8/8/r7 b - -",
		"1k6/6R1/1P6/5K1p/5P1r/8/8/8 b - -",
		"8/6k1/K1PR3p/8/8/8/8/1r6 b - -",
		"rnbqkb1r/pppppppp/5n2/8/8/5N2/PPPPPPPP/RNBQKB1R w KQkq -",
		"2r1k2r/2qb1pb1/1p2p1pn/nP1pP2p/P2B3P/3B1N1Q/3N1PP1/R4RK1 w k -",
		"rn2r1k1/1pp1p1bp/p3Rpp1/8/3q4/5B2/P1PN1PPP/R2Q2K1 w - -",
		"rnbqkbnr/ppp1pppp/8/3p4/5P2/5N2/PPPPP1PP/RNBQKB1R b KQkq -",
		"4qr2/6kp/2r1n1p1/2nRp1P1/4P2P/2p1B1Q1/2P3BK/5R2 b - -",
		"8/6pk/7p/1p1rpq2/1P2R3/Pp3P1P/5QPK/8 w - -",
		"r7/8/1R1pk1p1/4P2p/2p2K1P/5PP1/8/8 b - -",
		"rn1q1rk1/p3ppbp/1p1p2p1/8/2PN4/1PB1P1P1/P4PKP/2RQ1R2 b - -",
		"1rbqk1nr/3nppbp/p2p2p1/1p2P3/3BBP2/2N2N2/PPP3PP/R2QK2R b KQk -",
		"r4r1k/5ppp/8/p1q1p3/1p1nP3/P2B4/1PPQ1RPP/R5K1 w - -",
		"8/3k4/1P4p1/p2P3p/P1K1p2P/8/6P1/8 b - -",
		"r1bqk1nr/pp1pppbp/2n3p1/2p5/2P5/2N2NP1/PP1PPPBP/R1BQK2R b KQkq -",
		"6r1/4pr1k/pQ1pR1Np/1b1P3n/1Pp2P2/6PP/P2qR1BK/8 b - -",
		"1q2r1k1/3r1p1p/1p1pp1p1/pPn1b1B1/2B1P3/1P2QPP1/P6P/1R1R2K1 w - -",
		"rnbqkb1r/ppp2ppp/4pn2/8/2pP4/5NP1/PP2PPBP/RNBQK2R b KQkq -",
		"r1b1k2r/1pqpbppp/p1n1pn2/8/3NP3/P1N1B3/1PP1BPPP/R2QK2R w KQkq -",
		"5bk1/1b1q2p1/p3p3/3pBpp1/Pp1P4/1PrBP2P/5P2/3Q2RK w - -",
		"rn1qkbnr/pp2ppp1/2p3bp/8/3P3P/5NN1/PPP2PP1/R1BQKB1R b KQkq -",
		"4r3/p3kp2/2qb1p1B/1pnR4/8/P1P2P2/1PQ5/2K4R w - -",
		"r2qk2r/pp1b1ppp/4pn2/2b5/8/P2B2N1/1PP2PPP/R1BQ1RK1 w kq -",
		"2kr1bnr/ppp1qp2/3p2p1/3Pn3/2P1PPPp/2N4P/PP2B3/R1BQK2R b KQ f3",
		"8/5pk1/p7/1p1pr1P1/2p4P/P1P5/1P3RP1/7K b - -",
		"4r3/2r3k1/4R1p1/4Kp1p/P1B2P2/bP1R2P1/7P/8 b - -",
		"5rk1/2p4p/2b5/pp6/1P2pP2/P1N3P1/5K1P/3R4 b - -",
		"r7/pp3Q2/2np2pk/2p1pb1q/8/PBB1P3/1P1KN3/8 w - -",
		"3r4/6p1/pN1r1p2/Pbk1p3/7p/1K1P1P2/3R2PP/3R4 w - -",
		"2r5/1p5p/4kp2/1p1p1R2/6P1/1P1KP2P/P7/8 b - -",
		"1Q6/5pk1/5Np1/7p/4PP2/8/2n2KPP/q7 w - -",
		"r1bqk2r/pp1nbppp/2p1pn2/3p2B1/2PP4/2NBPN2/PP3PPP/R2QK2R b KQkq -",
		"2rq1rk1/p3bppp/B1p5/2pp1b2/3Pn3/1P2PN2/PB3PPP/R2Q1RK1 b - -",
		"1r1r2k1/pbpnqppp/2n1pb2/2Pp4/Pp1P3P/1P1N1NP1/1BQ1PPB1/3RR1K1 b - h3",
		"r1bq1rk1/ppp3pp/2n1p3/3pPp2/2PPn3/P1PQ2P1/4NP1P/R1B1KB1R b KQ -",
		"1rr3k1/p3pp2/3p2p1/1qpP2N1/4P1n1/6P1/PPQB1P2/1R4K1 w - -",
		"6n1/bp3pk1/2p3p1/p1q1p2p/P3P3/1PP2BP1/3Q1PKP/2B5 w - -",
		"r1b1kbnr/1pp3pp/5p2/p1n1p3/P1B1P3/2P2N2/1P3PPP/RNB2RK1 w - -",
		"5rk1/4qpp1/1r2pn1p/8/Q2P4/P3P1B1/5PPP/2R2K2 w - -",
		"3R4/2q2pp1/p1k2b2/1n3Q2/1p1p4/1P4P1/PB3P2/1K2R3 b - -",
		"1q1rr1k1/1p3pp1/pbn3bp/3p4/P2N4/1PP1B2P/3QBPP1/3RRK2 w - -",
		"rnbqkbnr/pp2pppp/2p5/3p4/2PP4/2N5/PP2PPPP/R1BQKBNR b KQkq -"
		};
		return fens;
	}

	std::vector<std::string> benchFens3() {
		std::vector<std::string> fens = {
			"1r4k1/2R4p/5Kp1/1PB5/4P1PP/2b5/8/8 w - - 0 1",
			"q6k/5p2/p1N1p1p1/RPP1P1P1/4QP1P/1r6/6K1/8 b - - 0 1",
			"8/8/8/3N4/4p3/4K3/8/1k6 w - - 0 1",
			"8/6k1/1q3p1p/5p1P/2p5/p1Bp1B2/6P1/1K2R3 w - - 0 1",
			"4q2k/p1Q2rp1/1p3p1p/3N1p1n/4n3/PP2BR1P/6P1/6K1 w - - 0 1",
			"2Q5/8/1k1r2p1/1pp5/8/4K3/6PP/8 w - - 0 1",
			"1rbqk1nr/pp3pbp/3pp1p1/2p5/P2nP3/2NPBNP1/1PPQ1PBP/R3K2R b KQk - 0 1",
			"8/2k1n3/p6p/1p3n2/1P3P2/P1P3p1/1KB5/7R b - - 0 1",
			"rnbqk1nr/pp2ppb1/2p3p1/3pP2p/3P1P2/2NB4/PPP3PP/R1BQK1NR b KQkq - 0 1",
			"1b1r2k1/p2pqpp1/2r1pn1p/1Bp5/4NP1Q/P3P3/1P1R2PP/5RK1 b - - 0 1",
			"8/8/3b4/5B2/8/p1k5/K7/8 w - - 0 1",
			"8/8/8/4KQ2/4P3/8/2k5/5q2 b - - 0 1",
			"8/8/8/1k1r4/6PK/8/8/2R5 b - - 0 1",
			"4r3/8/8/8/R1k5/8/1K6/8 b - - 0 1",
			"8/4R3/k7/4p3/2p4p/2P4n/8/5K2 b - - 0 1",
			"8/8/8/8/4N3/7p/7K/k7 w - - 0 1",
			"8/Pb5B/4k3/4n3/4p1pp/2B1P3/5PK1/8 w - - 0 1",
			"rnbqk2r/ppp1ppb1/1n4pp/6B1/3P4/2N2N2/PP2PPPP/R2QKB1R w KQkq - 0 1",
			"2q1n1k1/3n1pbp/1pp3p1/p2p2N1/P2P1B2/2N4P/1PPQ1PP1/6K1 w - - 0 1",
			"r4rk1/pp2n1pp/1bn2p2/q2p4/8/P2QPNB1/1P1N1PPP/R4RK1 b - - 0 1",
			"8/1p4p1/6k1/5q2/5P1p/1P3RP1/3K4/8 w - - 0 1",
			"2rq1rk1/pp2ppbp/3n2p1/2p5/2nP4/1Q2PNP1/PB3PBP/1RR3K1 w - - 0 1",
			"1r3rk1/1bqpbpp1/npp1p2p/p3P3/2PP4/P1N2NP1/1PQ2PBP/R3R1K1 w - - 0 1",
			"8/P7/1KB5/8/4p2k/2n5/1N3p2/8 w - - 0 1",
			"8/8/2p1k2p/7N/5PP1/b6P/2P3K1/8 b - - 0 1",
			"8/8/5k2/5q1p/7P/5BP1/4RK2/8 b - - 0 1",
			"rnbq1rk1/1pp1bppp/pn1p4/3Pp3/1PB1P3/2N1BN1P/P1P2PP1/R2Q1RK1 w - - 0 1",
			"1r1r2k1/4bpp1/p1n1p2p/P1pqP3/1p1P3P/1P3NP1/Q4PK1/B1RR4 b - - 0 1",
			"rnbqkb1r/ppp1pppp/3p1n2/8/3PP3/2N5/PPP2PPP/R1BQKBNR b KQkq - 0 1",
			"8/5k2/r3q3/P4pR1/1P1B1R2/7P/5PPK/8 b - - 0 1",
			"5k2/1R6/1p2N3/1P2n1K1/6P1/8/8/4r3 b - - 0 1",
			"8/8/2K3pk/8/6RP/8/3r4/8 b - - 0 1",
			"1kb4r/1p3qp1/1R3p1p/p7/rN1NP3/8/1Q3PPP/2R4K b - - 0 1",
			"8/4k3/p7/5R2/8/P7/r5r1/1K3R2 b - - 0 1",
			"5bk1/2n1p3/p3Pp1p/1rp2BpP/2N3P1/2PK4/1P3P2/R7 w - - 0 1",
			"3q1r2/1pp2pbk/r1n1b1pp/pN1np3/P7/3P1NP1/1P1BPPBP/R1QR2K1 w - - 0 1",
			"r1bq1rk1/p1pnppbp/1p1p1np1/8/3P3B/2PBP3/PP1NNPPP/R2QK2R b KQ - 0 1",
			"r1bqkb1r/3npp1p/pP1p1np1/2pP4/8/2N2N2/PP2PPPP/R1BQKB1R w KQkq - 0 1",
			"r1bqkb1r/pp2pppp/2n2n2/3p4/2PP4/2N5/PP3PPP/R1BQKBNR w KQkq - 0 1",
			"6r1/2k5/8/1p1Pp2p/p1p1Pp1q/P1R1bP1P/1P2N3/5Q1K w - - 0 1",
			"5rk1/2P5/p7/5R2/1KPBp3/1P3b2/8/8 b - - 0 1",
			"6k1/3n1p1p/2q3p1/4p2P/2B1P3/rp2QP2/5P2/1K2N1R1 b - - 0 1",
			"8/8/3K4/6R1/k7/4N3/8/2r5 w - - 0 1",
			"r1b1k2r/2qpbppp/p1n1pn2/3N4/Pp2P3/1N6/1PPQBPPP/R1BR2K1 b kq - 0 1",
			"rnbq1rk1/4bppp/3p1n2/1pp1p3/p3P3/2PP1N2/PPBN1PPP/R1BQR1K1 w - - 0 1",
			"1q1r4/4rppk/pNNp1b1p/P7/4bP2/2R3QP/1P3KP1/2R5 b - - 0 1",
			"r1bqkb1r/pp1npppp/2n5/2ppP3/5P2/3P1N2/PPP3PP/RNBQKB1R w KQkq - 0 1",
			"8/8/8/6p1/P4rP1/2R4k/8/3K4 b - - 0 1",
			"rnbqkbnr/ppp2ppp/4p3/3p4/3PP3/2N5/PPP2PPP/R1BQKBNR b KQkq - 0 1",
			"r1b4r/ppp1k2p/3p1np1/4ppn1/2PPP3/P1PB1P2/6PP/R1B2RK1 w - - 0 1",
			"4r3/p1qb2kp/2pp1r2/2n2pp1/2P5/4P1P1/PRQ1NPBP/1R4K1 b - - 0 1",
			"8/8/8/7p/4k3/8/8/K3N3 w - h6 0 1",
			"7q/7P/8/6pR/3k2P1/6N1/5K2/8 w - - 0 1",
			"8/r7/PR1bk3/6p1/8/8/5P2/R4K2 w - - 0 1",
			"8/1P3pk1/7p/6p1/3q4/1RR5/8/K7 b - - 0 1",
			"1rb2rk1/p1q2ppp/2pbpn2/8/2B1P3/2N1B2P/PPQ2PP1/R4RK1 b - - 0 1",
			"1k1n4/8/1p1p2p1/1PpPp1P1/2P1P3/3K4/5B2/8 w - - 0 1",
			"1k6/1b5p/p3p1p1/2rpq2n/6R1/R4P2/2PQ3P/3K1B2 w - - 0 1",
			"r1b2rk1/ppp3bp/n2p2q1/3PppBn/2P5/P1N2N2/1P2BPP1/R2QK2R w KQ - 0 1",
			"8/6q1/8/p1p5/1p2N3/1P1P4/1kP1RK2/8 w - - 0 1",
			"8/4k3/7p/p1Rr4/5pp1/P3b1P1/4Qn1P/3q1BK1 b - - 0 1",
			"2N5/8/8/8/7p/7K/8/k7 b - - 0 1",
			"3nr2k/5pp1/4p2p/r1p1PP1P/2Pp1RP1/pP1N4/P2K4/6R1 b - - 0 1",
			"r4rk1/p2n1ppp/b1p1p3/2bpP3/2P4q/1P1B4/PBQN1PPP/R4RK1 w - - 0 1",
			"8/3b4/3k4/1p3p2/1P1K1P2/8/8/5B2 b - - 0 1",
			"r1bqk2r/ppp3pp/2np1n2/1Bb1p3/2Q1P3/5N2/PPP2PPP/RNB2RK1 b kq - 0 1",
			"4r1k1/1p1bq1pn/r2p1p2/p1pP2n1/4P2Q/2N5/PPP3B1/K3NR1R w - - 0 1",
			"8/5pk1/3p1qp1/1ppPp2Q/P1b1P3/2P2P2/2B3R1/6K1 w - - 0 1",
			"8/7R/4k3/1N6/3PP1K1/8/r7/8 b - - 0 1",
			"8/8/3B4/5k2/8/7p/6r1/6K1 w - - 0 1",
			"8/4pk1p/6p1/1R6/1p3P2/1PbpK2P/6P1/8 b - - 0 1",
			"8/8/4r1p1/1R5p/1P4kP/P7/1K6/8 w - - 0 1",
			"r1bq1rk1/pp3pbp/n2p1np1/2pPp3/2P1P1P1/P1NBB2P/1P2NP2/R2QK2R b KQ - 0 1",
			"8/2k1p3/6p1/1p3n2/1P1PQP1p/3P3P/6PK/2r2r2 w - - 0 1",
			"r1b2rk1/pp3ppp/2p5/4n3/3NPP2/8/PP2B1PP/R2R2K1 b - f3 0 1",
			"r4rk1/4ppb1/2q1b1pp/3n4/2p5/N1P3NP/1PQ2PP1/R1BR2K1 b - - 0 1",
			"rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 0 1",
			"n4rk1/1r3pbn/b2p2p1/p1pPp2P/N1P1P2q/P1QBBP1P/4N1K1/R6R b - - 0 1",
			"2kr1b1r/ppqn1pp1/4p1p1/8/8/6P1/PPP1QPBP/R1B2RK1 b - - 0 1",
			"r1bq1rk1/pp1p1pbp/2n1pnp1/8/1PP5/2N2NP1/3PPPBP/1RBQK2R b K - 0 1",
			"3k4/3p4/8/PQ3q2/2P1pppP/8/7K/7N b - - 0 1",
			"8/3q3k/1p2rppp/p1p1b3/P1N4R/1P1rPQP1/3P3P/5R1K b - - 0 1",
			"8/1p2r3/p2R2kp/4nnp1/8/P4P2/1PB3P1/1K6 b - - 0 1",
			"5rk1/Rp2p3/3p2pq/2nPp1b1/2P1P1Q1/6P1/5NN1/5K2 b - - 0 1",
			"8/8/4p2R/4r2p/6p1/2k3P1/5P2/6K1 b - - 0 1",
			"r1b2rk1/ppp3bp/5q2/2pPpp2/2P1P3/2N3PP/PP2QPK1/R5NR b - - 0 1",
			"8/1pr3p1/p1rpk2p/P3p3/R1Pp1P2/3P4/1P4PP/4R1K1 b - - 0 1",
			"rnb2rk1/ppp2pp1/5q1p/2pQp3/8/2P2NP1/PP2PPBP/R3K2R b KQ - 0 1",
			"8/7p/8/5q2/kpp2P2/4B2P/6QK/8 w - - 0 1",
			"4q3/3b4/3b3k/1ppPp2p/p3Pp2/P2B1NrP/1PR2QP1/7K b - - 0 1",
			"8/4kBb1/2pp1p2/p1p2P1p/P1P1KP2/1P5r/1R5P/8 w - - 0 1",
			"5r2/p7/4n3/1p2Pk2/1P1bR3/P4PBP/5PK1/8 b - - 0 1",
			"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
			"7R/8/6k1/8/8/8/3K4/5r2 b - - 0 1",
			"5k2/8/3R2p1/3K4/3B1PpP/7r/8/8 b - - 0 1",
			"6r1/k5q1/8/1p1Pp3/p1p1Pp1p/P3bP1Q/1PR1N3/7K w - - 0 1",
			"2R5/p2r2p1/1p6/3k1P2/6P1/6KP/P7/8 w - - 0 1",
			"rnq1k2r/pp2ppbp/5np1/2p1N3/8/1P6/PBPP1PPP/RN1Q1RK1 w kq - 0 1",
			"8/7p/p2Ppbp1/2k5/P1N1K3/7P/6P1/8 w - - 0 1",
			"r5k1/2Rr2pp/3n1pq1/pp1Ppb2/7P/PQ6/1P2BPP1/3RB1K1 w - - 0 1",
			"r3qrk1/3bp2p/2p1n1p1/p1Pp1p2/NP2n3/P3PNP1/1RQ2PBP/5RK1 b - - 0 1",
			"8/4n3/p3k2p/1p3n2/1P3P2/P1P3p1/2B5/2K4R w - - 0 1",
			"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
			"r1b1r1k1/pp3pbp/1q1p1np1/2pP4/1nP5/P1N2N2/1P1B1PPP/RB1Q1RK1 b - - 0 1",
			"4nr1k/pppr2pp/1n6/3P1p1q/P1PP4/1Q1BBP2/7P/4RRK1 b - - 0 1",
			"r1bq1rk1/1p2ppbp/1n1p1np1/p1pP4/P3PP2/1BN1BN2/1PP3PP/R3QRK1 w - 0 1",
			"8/8/8/6bp/8/8/1k4K1/8 w - - 0 1",
			"r4b1k/r4pp1/2pp3p/2q2N1P/pp2PP2/4R3/PPP1Q1P1/1K1R4 w - - 0 1",
			"5k2/8/6Q1/1pp4p/2q2N1P/8/5PK1/8 w - - 0 1",
			"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
			"1r3bk1/4pp2/3p3p/3P4/Q1p2Nn1/1r6/1PK3PP/1R6 w - - 0 1",
			"8/4R3/1pBp4/p2P1pp1/5k2/P7/3r3P/7K b - - 0 1",
			"8/2r5/3k4/pB1b1p1p/Prp3pP/2R1P1P1/5P2/3RK3 b - - 0 1",
			"6k1/2R5/7p/p2r4/4npp1/P3b1P1/1B2Q2P/3q1B1K w - - 0 1",
			"6k1/5p2/3p3p/PP4pP/3b4/8/4KR2/7q b - - 0 1",
			"4r3/8/8/8/8/2k5/8/K2R4 w - - 0 1",
			"r5k1/2p2qp1/3p1rnp/p1pb4/P1P1P1R1/1P2Q1RP/3N2P1/6K1 w - - 0 1",
			"r2qk2r/1bppbpp1/np2p2p/p7/2PPP3/2N2NP1/PP3PBP/R2Q1RK1 w kq - 0 1",
			"r1q1rn1k/4bQpn/2pp4/1p2P1P1/p2P3R/P1NBB2b/1P1N4/R5K1 w - - 0 1",
			"2R5/6pk/2P2p1p/p1r2K1P/Pb2PP1B/8/8/8 w - - 0 1",
			"4r1k1/1pnr2pp/pb3p2/3b4/PP1P4/2N5/4NBPP/R2R2K1 w - - 0 1",
			"2rqr1k1/1b2bp1p/pp3np1/2pp4/3P4/PP2P3/1BRNBPPP/3Q1RK1 b - - 0 1",
			"rnbqk2r/ppppp1bp/5np1/5p2/2PP4/5NP1/PP2PP1P/RNBQKB1R w KQkq - 0 1",
			"r3r2k/1pp2R1n/p1bp4/5P2/4P2q/2N1NQ2/PPP4P/1K4R1 w - - 0 1",
			"r1bqkb1r/p2pnppp/2p1p3/2p5/4P2N/3P4/PPP2PPP/RNBQK2R b KQkq - 0 1",
			"1Q6/P6k/6p1/8/3q4/7p/8/1K6 b - - 0 1",
			"4qr1k/5ppn/7p/2R1pP2/5b1r/5PN1/B1Q2PKP/1R6 b - - 0 1",
			"rn3rk1/pp2ppbp/6p1/3B4/3P4/2N3Pq/PP2PP2/R1BQK3 w Q - 0 1",
			"r2qr1k1/pb3pp1/1p3n1p/1PpNb3/2B5/PQ2PN2/5PPP/3R1RK1 b - - 0 1",
			"7K/8/8/8/4Q3/5r2/6k1/6n1 b - - 0 1",
			"3b3r/6k1/1p2qp1r/p1p1p1p1/PnNPB1b1/1PB1P1P1/3R1Q1P/4R1K1 b - - 0 1",
			"5k2/4ppbp/r5p1/Pp1pP3/1p1N1P2/4P2P/1P4P1/R5K1 b - f3 0 1",
			"7k/2rp1Qp1/p3p2p/4Pp2/P1p1bP2/6R1/1r4PP/4K3 b - - 0 1",
			"2qrr1k1/4bppp/p1n1bn2/1p1pN3/8/P1NB3P/1PPQRPPB/4R1K1 b - - 0 1",
			"4n1k1/4r2p/6p1/3Q2PP/6P1/6K1/8/8 b - - 0 1",
			"r1q1rn1k/4bQpB/2pp4/1p2P1P1/p2P3R/P1N1B2b/1P1N4/R5K1 b - - 0 1",
			"6k1/pp2qpp1/3npb1p/1Q6/1P6/1Br1PN1P/P4PP1/3R2K1 w - - 0 1",
			"rn1qkb1r/pbpppppp/1p3n2/8/2PP4/2N2N2/PP2PPPP/R1BQKB1R b KQkq d3 0 1",
			"b7/P5pk/7p/5p1q/5P2/8/3Q2NK/8 w - - 0 1",
			"3r3k/p3bp2/P1p1p3/2p2r1p/2P1Np1P/1P1P1P2/2K2PR1/7R b - - 0 1",
			"8/3Q1kb1/2r2n2/p1q1p1P1/1p2P3/8/1PP2R1K/5R2 b - - 0 1",
			"q7/3k4/1KR5/1R6/8/8/8/8 w - - 0 1",
			"8/3Q1pk1/3bp1p1/3b2P1/1P2n3/3BPN2/5P2/r5K1 w - - 0 1",
			"8/8/P7/8/7k/R3K1p1/5r2/8 b - - 0 1",
			"3R4/1r6/2p4P/4Pp2/1P6/4k1K1/8/8 b - - 0 1",
			"5r2/1p4kp/2p1r3/2P1q3/P2Q1p2/2N3PP/5P2/3R2K1 w - - 0 1",
			"rnbqk2r/ppp1bp1p/4p3/5p2/3PN3/5N2/PPP2PPP/R2QKB1R w KQkq - 0 1",
			"8/p3n3/1p2k3/2pb4/B4B1p/P4P2/r5PP/3R2K1 w - - 0 1",
			"8/8/8/6k1/3B1b2/4K3/2B5/8 w - - 0 1",
			"r2qkbnr/p1p2p1p/2bp2p1/2p5/4P3/2N1NQ2/PPP2PPP/R1B1R1K1 b kq - 0 1",
			"r1b2rk1/p2p1ppp/1p1qp3/1B2N2Q/1P1nP3/7R/P2P1PPP/5RK1 b - - 0 1",
			"R7/P5k1/5p2/8/6p1/4P1P1/r4P2/4K3 w - - 0 1",
			"rn2kb1r/1B1q1p2/7p/1pp3p1/3p4/NPnQ2P1/P1N1PP1P/R1B2RK1 b kq - 0 1",
			"1r6/3qb1k1/1p3p1r/p1p1p1p1/PnN1B3/1PBPP1Pb/5Q1P/R2R2K1 b - - 0 1",
			"8/5p2/2pk4/p7/Pn1P2r1/1NR3p1/6P1/6K1 w - - 0 1",
			"8/8/8/4k3/1B6/8/3r4/K7 b - - 0 1",
			"r2q1rk1/1bp2ppp/n2p4/4Pn2/1p3P2/2N5/BPPQ1BPP/R4RK1 w - - 0 1",
			"4r1k1/1pp2p2/1n1p2pp/r2P4/p3PP1q/PBP2P1P/2QBR3/5RK1 b - - 0 1",
			"rn1q1rk1/1p1bbppp/1np5/p2p4/P7/2NP2PP/1PP1NPB1/R1BQR1K1 w - - 0 1",
			"3Q4/5r1k/7n/8/3B4/5r2/3K4/8 b - - 0 1",
			"1r1qr1k1/pb1p1p1p/1pn1p1p1/1Np2n2/2P2N2/1P2P1P1/PR1P1PBP/3QR1K1 b - - 0 1",
			"8/5pk1/2p5/1p1p1pP1/1P1P1P1p/2P1KR1P/5R2/2r4r w - - 0 1",
			"8/1k6/2pQ4/r7/3P4/2K5/8/8 w - - 0 1",
			"rn4Q1/3pkp1p/p3p3/1p2P3/7R/2N3P1/PPP4P/2q2K2 w - - 0 1",
			"3b4/B4pk1/1p4pp/1Pq5/5PQ1/R3p1PP/2R3K1/8 b - - 0 1",
			"rnbqkbnr/1p1p1ppp/p3p3/8/3NP3/2N5/PPP2PPP/R1BQKB1R b KQkq - 0 1",
			"5r1k/1p5p/2p1r3/2P1q3/P2Q4/6NP/5P2/3R2K1 b - - 0 1",
			"r1bqkb1r/pp2pppp/2np1n2/8/2BNP3/2N5/PPP2PPP/R1BQK2R b KQkq - 0 1",
			"8/2r5/3k4/pB1b1p1p/Prp3pP/2R1P1P1/5P2/2R1K3 w - - 0 1",
			"r1b2rk1/ppp2ppp/7n/8/2BqP3/5N2/PP3PPP/R4RK1 w - - 0 1",
			"B7/8/8/2k5/3b4/3P4/8/3K4 w - - 0 1",
			"3r2k1/pb3pbp/1Pn5/7p/2p5/2N1P1P1/1P3PBP/R1B3K1 b - - 0 1",
			"8/6k1/6p1/1pR3Pp/8/1r5P/6K1/8 w - - 0 1",
			"r3kbnr/pp1q1ppp/2n1p3/1Bp5/4Q3/2P5/PP1P1PPP/RNB1K2R b KQkq - 0 1",
			"1r4k1/1p3r1p/p5p1/8/4Q3/1NP3b1/PP4Pq/R2BRK2 w - - 0 1",
			"8/8/8/5r2/8/2k5/1R6/1K6 b - - 0 1",
			"8/7K/8/5k2/r7/8/8/B7 w - - 0 1",
			"8/8/7k/4B3/4K3/6R1/8/4r3 w - - 0 1",
			"r1r3k1/pbqnpp1p/1p1p1n1b/2p3p1/PP1PP2B/2PB1P2/3NN1PP/R2Q1R1K w - - 0 1",
			"rn1q1rk1/pb1p1ppp/1p2pn2/6B1/1bPp4/PQN1PN2/1P3PPP/R3KB1R w KQ - 0 1",
			"3k3r/1pb1n1p1/4Q3/1p1P2P1/8/4B3/q1B1KP2/2R5 w - - 0 1",
			"4r2k/3n3p/2Q1b2P/p3P3/1p3rP1/2P3N1/P1P4R/1K6 w - - 0 1",
			"8/1r3k2/8/5P2/5K2/8/2R5/8 w - - 0 1",
			"r1bqkb1r/pp1ppppp/2n2n2/2p5/2P5/6P1/PP1PPPBP/RNBQK1NR w KQkq - 0 1",
			"5r1k/1p2r2p/p4n1q/3pBB2/PP3R2/4P3/1Q5P/6RK w - - 0 1",
			"rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq d3 0 1",
			"r1bqk2r/ppppbppp/2n2n2/4p3/2B1P3/3P1N2/PPPN1PPP/R1BQK2R b KQkq - 0 1",
			"r7/8/6p1/1p2kp1p/1P1n3P/P1N3P1/1K6/3R4 w - - 0 1",
			"r1b1k2r/2qnbppp/p2p1n2/4p1B1/Pp2P3/2N2P2/1PPN2PP/R2QKB1R w KQkq - 0 1",
			"1r3k2/2p2n2/3p4/p1pP3p/P1P2N2/1P4rP/2Q3PK/8 b - - 0 1",
			"Q7/8/7p/8/3K4/7P/2q2k2/8 b - - 0 1",
			"rnbq1rk1/pp4bp/2pp1np1/4pp2/1PP5/2NP1NP1/PB2PPBP/R2Q1RK1 b - - 0 1",
			"r4k1r/pp1bnpp1/2n1p1q1/3pP2p/2pP4/P1P2N1Q/3B1PPP/R1RB2K1 w - h6 0 1",
			"Q7/4p1k1/5bpp/P7/1n6/7P/q4PP1/5RK1 w - - 0 1",
			"8/8/5n2/5p1k/7P/8/4K1N1/8 b - - 0 1",
			"8/7p/6p1/2PkNb2/5P2/4P1KP/8/8 w - - 0 1",
			"8/8/8/8/5K2/8/4rk2/8 w - - 0 1",
			"8/8/1k6/1p6/pK1nN3/8/8/8 w - - 0 1",
			"3r4/4k3/6p1/1p3p1p/1P1n3P/P1N3P1/1K1R4/8 b - - 0 1",
			"r6r/1p3kb1/1np1p3/p2pPnQp/P2P1P2/2N1B3/1PP1BP2/2K5 b - - 0 1"
		};
		return fens;
	}


	int64_t bench(int depth, int64_t& totalTime) {
		return bench(benchFens1(), depth, totalTime);
	}

	int64_t bench2(int depth, int64_t& totalTime) {
		return bench(benchFens2(), depth, totalTime);
	}

	int64_t bench3(int depth, int64_t& totalTime) {
		return bench(benchFens3(), depth, totalTime);
	}

	int64_t bench(std::vector<std::string> fens, int depth, int64_t& totalTime) {
		std::cout << "Benchmark" << std::endl;
		std::cout << "------------------------------------------------------------------------" << std::endl;
		int64_t totalNodes = 0;
		int64_t totalQNodes = 0;
		totalTime = 0;
		double avgBF = 0.0;
		std::cout << std::setprecision(3) << std::left << std::setw(4) << "Nr" << std::setw(7) << "Time" << std::setw(10) << "Nodes" << std::setw(6) << "Speed" << std::setw(6) << "BF" << std::setw(6) << "TT[%]"
			<< std::setw(40) << "PV" << std::endl;
#pragma warning(suppress: 26409)
		std::unique_ptr<Search> srch = std::make_unique<Search>();
		for (int i = 0; i < gsl::narrow_cast<int>(fens.size()); i++) {
			Position pos(fens.at(i));
			srch->PrintCurrmove = false;
			//srch.uciOutput = false;
			srch->NewGame();
			srch->timeManager.initialize(TimeMode::FIXED_DEPTH, depth);
			srch->Think(pos);
			int64_t endTime = now();
			totalTime += endTime - srch->timeManager.GetStartTime();
			srch->NodeCount *= settings::parameter.HelperThreads + 1;
			srch->QNodeCount *= settings::parameter.HelperThreads + 1;
			totalNodes += srch->NodeCount;
			totalQNodes += srch->QNodeCount;
			avgBF += srch->timeManager.GetEBF(depth) * (srch->NodeCount - srch->QNodeCount);
			const int64_t runtime = endTime - srch->timeManager.GetStartTime();
			int64_t rt = runtime;
			if (rt == 0) rt = 1;
			std::cout << std::left << std::setw(4) << i << std::setw(7) << runtime << std::setw(10) << srch->NodeCount << std::setw(6)
				<< srch->NodeCount / rt << std::setw(6) << srch->timeManager.GetEBF(depth) << std::setw(6) << (settings::parameter.HelperThreads ? 0 : 100.0 * tt::GetHitCounter() / tt::GetProbeCounter())
				<< std::setw(40) << srch->PrincipalVariation(pos, depth) << std::endl;
		}
		avgBF = avgBF / (totalNodes - totalQNodes);
		std::cout << "------------------------------------------------------------------------" << std::endl;
		std::cout << std::setprecision(5) << "Total:  Time: " << totalTime / 1000.0 << " s  Nodes: " << totalNodes / 1000000.0 << " - " << totalQNodes / 1000000.0 << " MNodes  Speed: " << totalNodes / totalTime << " kN/s  "
			"BF: " << std::setprecision(3) << avgBF << std::endl;

		return totalNodes;
	}


	uint64_t nodeCount = 0;

	uint64_t perft(Position& pos, int depth) {
		nodeCount++;
		if (depth == 0) return 1;
		ValuatedMove move;
		uint64_t result = 0;
		ValuatedMove* moves = pos.GenerateMoves<ALL>();
		while ((move = *moves).move) {
			Position next(pos);
			if (next.ApplyMove(move.move)) result += perft(next, depth - 1);
			++moves;
		}
		return result;
	}

	uint64_t perft1(Position& pos, int depth) {
		nodeCount++;
		if (depth == 0) return 1;
		ValuatedMove move;
		uint64_t result = 0;
		ValuatedMove* moves = pos.GenerateMoves<TACTICAL>();
		while ((move = *moves).move) {
			Position next(pos);
			if (next.ApplyMove(move.move)) result += perft1(next, depth - 1);
			++moves;
		}
		moves = pos.GenerateMoves<QUIETS>();
		while ((move = *moves).move) {
			Position next(pos);
			if (next.ApplyMove(move.move)) result += perft1(next, depth - 1);
			++moves;
		}
		return result;
	}

	uint64_t perft2(Position& pos, int depth) {
		nodeCount++;
		if (depth == 0) return 1;
		ValuatedMove move;
		uint64_t result = 0;
		ValuatedMove* moves = pos.GenerateMoves<WINNING_CAPTURES>();
		while ((move = *moves).move) {
			Position next(pos);
			if (next.ApplyMove(move.move)) result += perft2(next, depth - 1);
			++moves;
		}
		moves = pos.GenerateMoves<EQUAL_CAPTURES>();
		while ((move = *moves).move) {
			Position next(pos);
			if (next.ApplyMove(move.move)) result += perft2(next, depth - 1);
			++moves;
		}
		moves = pos.GenerateMoves<LOOSING_CAPTURES>();
		while ((move = *moves).move) {
			Position next(pos);
			if (next.ApplyMove(move.move)) result += perft2(next, depth - 1);
			++moves;
		}
		moves = pos.GenerateMoves<QUIETS>();
		while ((move = *moves).move) {
			Position next(pos);
			if (next.ApplyMove(move.move)) result += perft2(next, depth - 1);
			++moves;
		}
		return result;
	}

	uint64_t perft3(Position& pos, int depth) {
		nodeCount++;
		if (depth == 0) return 1;
		uint64_t result = 0;
		pos.InitializeMoveIterator<MAIN_SEARCH>(nullptr, nullptr, nullptr, nullptr, MOVE_NONE);
		Move move;
		while ((move = pos.NextMove())) {
			Position next(pos);
			if (next.ApplyMove(move)) {
				result += perft3(next, depth - 1);
			}
		}
		return result;
	}

	uint64_t perft4(Position& pos, int depth) {
		nodeCount++;
		uint64_t result = 0;
		const ValuatedMove* moves = pos.GenerateMoves<LEGAL>();
		const int movecount = pos.GeneratedMoveCount();
		if (depth == 1) return movecount;
		for (int i = 0; i < movecount; ++i) {
			Position next(pos);
			next.ApplyMove(moves[i].move);
			result += perft(next, depth - 1);
		}
		return result;
	}

	uint64_t perftcomb(Position& pos, int depth) {
		nodeCount++;
		if (depth == 0) return 1;
		uint64_t result = 0;
		pos.InitializeMoveIterator<MAIN_SEARCH>(nullptr, nullptr, nullptr, nullptr, MOVE_NONE);
		Move move;
		while ((move = pos.NextMove())) {
			Position next(pos);
			if (next.ApplyMove(move)) {
				const uint64_t result3 = perftcomb(next, depth - 1);
				const uint64_t result1 = perft(next, depth - 1);
				if (result1 != result3) {
					std::cout << "Error at Depth " << depth - 1 << "  " << next.fen() << std::endl;
					std::cout << "  R1: " << result1 << " R3: " << result3 << " Last Move " << toString(move) << std::endl;
					std::cout << "  Pos before: " << pos.fen() << std::endl;
					return result;
				}
				result += result3;
			}
		}
		return result;
	}

	void divide(Position& pos, int depth) {
		ValuatedMove* moves = pos.GenerateMoves<ALL>();
		ValuatedMove move;
		uint64_t total = 0;
		std::vector<ValuatedMove> vm;
		while ((move = *moves).move) {
			vm.push_back(move);
			++moves;
		}
		std::sort(vm.begin(), vm.end(), [](ValuatedMove a, ValuatedMove b) {return toString(a.move) > toString(b.move); });
		for (auto m : vm) {
			Position next(pos);
			if (next.ApplyMove(m.move)) {
				const uint64_t p = perft(next, depth - 1);
				std::cout << toString(m.move) << "\t" << p << "\t" << next.fen() << std::endl;
				total += p;
			}
		}
		std::cout << "Total: " << total << std::endl;
	}

	void divide3(Position& pos, int depth) {
		pos.InitializeMoveIterator<MAIN_SEARCH>(nullptr, nullptr, nullptr, nullptr, MOVE_NONE);
		Move move = MOVE_NONE;
		uint64_t total = 0;
		while ((move = pos.NextMove())) {
			Position next(pos);
			if (next.ApplyMove(move)) {
				const uint64_t p = perft3(next, depth - 1);
				std::cout << toString(move) << "\t" << p << "\t" << next.fen() << std::endl;
				total += p;
			}
		}
		std::cout << "Total: " << total << std::endl;
	}

	void testSearch(Position& pos, int depth) {
#pragma warning(suppress: 26409)
		Search* engine = new Search;
		engine->timeManager.initialize(TimeMode::FIXED_DEPTH, depth);
		const ValuatedMove vm = engine->Think(pos);
		std::cout << "Best Move: " << toString(vm.move) << " " << static_cast<int>(vm.score) << std::endl;
		delete engine;
	}

	void testRepetition() {
		Position pos("5r1k/R7/5p2/4p3/1p1pP3/1npP1P2/rqn1b1R1/7K w - - 0 1");
#pragma warning(suppress: 26409)
		Search* engine = new Search;
		engine->timeManager.initialize(TimeMode::FIXED_DEPTH, 5);
		const ValuatedMove vm = engine->Think(pos);
		std::cout << (((vm.move == createMove(G2, H2)) && (vm.score == VALUE_DRAW)) ? "OK     " : "ERROR ") << toString(vm.move) << "\t" << static_cast<int>(vm.score) << std::endl;
		delete engine;
	}

	void testFindMate() {
		std::map<std::string, Move> puzzles;
		//Mate in 2
		puzzles["2@2bqkbn1/2pppp2/np2N3/r3P1p1/p2N2B1/5Q2/PPPPKPP1/RNB2r2 w KQkq - 0 1"] = createMove(F3, F7);
		puzzles["2@8/6K1/1p1B1RB1/8/2Q5/2n1kP1N/3b4/4n3 w - - 0 1"] = createMove(D6, A3);
		puzzles["2@B7/K1B1p1Q1/5r2/7p/1P1kp1bR/3P3R/1P1NP3/2n5 w - - 0 1"] = createMove(A8, C6);
		puzzles["2@1r6/4b2k/1q1pNrpp/p2Pp3/4P3/1P1R3Q/5PPP/5RK1 w - - 0 1"] = createMove(H3, H6);
		puzzles["2@3r1b1k/5Q1p/p2p1P2/5R2/4q2P/1P2P3/PB5K/8 w - - 0 1"] = createMove(F7, G8);
		puzzles["2@r1bq2r1/b4pk1/p1pp1p2/1p2pP2/1P2P1PB/3P4/1PPQ2P1/R3K2R w - - 0 1"] = createMove(D2, H6);
		puzzles["2@8/8/8/r2p4/kp6/1R1Q4/8/K7 w - - 0 1"] = createMove(B3, B2);
		puzzles["2@7B/2R1KnQ1/1p1PP3/3k4/2N5/r3p1N1/4n3/1q6 w - - 0 1"] = createMove(G7, A1);
		//Mate in 3
		puzzles["3@1r3r1k/5Bpp/8/8/P2qQ3/5R2/1b4PP/5K2 w - - 0 1"] = createMove(E4, H7);
		puzzles["3@r1b1r1k1/1pq1bp1p/p3pBp1/3pR3/7Q/2PB4/PP3PPP/5RK1 w - - 0 1"] = createMove(H4, H7);
		puzzles["3@r5rk/5p1p/5R2/4B3/8/8/7P/7K w - - 0 1"] = createMove(F6, A6);
		puzzles["3@5B2/6P1/1p6/8/1N6/kP6/2K5/8 w - - 0 1"] = createMove<MoveType::PROMOTION>(G7, G8, KNIGHT);
		puzzles["3@8/R7/4kPP1/3ppp2/3B1P2/1K1P1P2/8/8 w - - 0 1"] = createMove(F6, F7);
		puzzles["3@3r1r1k/1p3p1p/p2p4/4n1NN/6bQ/1BPq4/P3p1PP/1R5K w - - 0 1"] = createMove(G5, F7);
		puzzles["3@rn4k1/3q2r1/p2P3Q/2p1p2P/Pp2P3/1P1B4/2P2P2/2K3R1 w - -"] = createMove(D3, C4);
		puzzles["3@6k1/5pp1/7p/3n4/8/2P5/KQq4P/4R3 b - - 0 1"] = createMove(D5, C3);
		//Mate in 4
		puzzles["4@r5r1/5b1k/2p2p1p/1pP2P2/1P1pRq2/8/1QB3PP/4R1K1 b - -"] = createMove(G8, G2);
		puzzles["4@2q1kr2/6R1/2p1p2Q/ppP2p1P/3P4/8/PP6/1K6 w - - 0 1"] = createMove(H6, G6);
		puzzles["4@r1k4r/4Pp1p/p2Q4/1p2p3/2q1b2P/P7/1PP5/2KR3R w - - 0 1"] = createMove(D6, D8);
		puzzles["4@8/5p1p/4p3/1Q6/P2k4/2p5/7q/2K5 b - - 0 1"] = createMove(H2, D2);
		//Mate in 5
		puzzles["5@6r1/p3p1rk/1p1pPp1p/q3n2R/4P3/3BR2P/PPP2QP1/7K w - -"] = createMove(H5, H6);
		puzzles["5@2q1nk1r/4Rp2/1ppp1P2/6Pp/3p1B2/3P3P/PPP1Q3/6K1 w - - 0 1"] = createMove(E7, E8);
#pragma warning(suppress: 26409)
		Search* engine = new Search;
		std::map<std::string, Move>::iterator iter;
		int count = 0;
		for (iter = puzzles.begin(); iter != puzzles.end(); ++iter) {
			engine->Reset();
			std::string mateIn = iter->first.substr(0, 1);
			std::string fen = iter->first.substr(2, std::string::npos);
			engine->timeManager.initialize(TimeMode::FIXED_DEPTH, 2 * atoi(mateIn.c_str()) - 1);
			Position pos(fen);
			const ValuatedMove vm = engine->Think(pos);
			std::cout << ((vm.move == iter->second) && (vm.score == VALUE_MATE - engine->timeManager.GetMaxDepth()) ? "OK    " : "ERROR ") << "\t" << toString(vm.move) << "/" << toString(iter->second)
				<< "\t" << static_cast<int>(vm.score) << "/" << static_cast<int>(VALUE_MATE) - engine->timeManager.GetMaxDepth() << "\t" << fen << std::endl;
			count++;
		}
		delete engine;
	}

	void testTacticalMoveGeneration() {
		std::vector<std::string> lines;
		std::ifstream s;
		s.open("C:/Users/chrgu_000/Desktop/Data/cutechess/testpositions/ccrl.epd");
		std::string l;
		if (s.is_open())
		{
			long lineCount = 0;
			while (s)
			{
				std::getline(s, l);
				lines.push_back(l);
				lineCount++;
				if ((lineCount & 8191) == 0) std::cout << ".";
				if (((lineCount & 65535) == 0) || !s) {
					std::cout << lineCount << " Lines read from File - starting to analyze" << std::endl;
					long count = 0;
					for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it) {
						std::string line = *it;
						if (line.length() < 10) continue;
						const size_t indx = line.find(" c0");
						std::string fen = line.substr(0, indx);
						Position p1(fen);
						ValuatedMove* tacticalMoves = p1.GenerateMoves<TACTICAL>();
						Position p2(fen);
						ValuatedMove* allMoves = p2.GenerateMoves<ALL>();
						ValuatedMove* tm = tacticalMoves;
						ValuatedMove* am = nullptr;
						Move move;
						//check move is really tactical
						while ((move = tm->move)) {
							Position p3(p1);
							if (p3.ApplyMove(move)) {
								if (p3.GetMaterialKey() == p1.GetMaterialKey()) {
									std::cout << std::endl << "Move " << toString(move) << " doesn't change Material key: " << fen << std::endl;
									//__debugbreak();
								}
								bool found = false;
								Move amove = MOVE_NONE;
								am = allMoves;
								while ((amove = am->move)) {
									if (amove == move) {
										found = true;
										break;
									}
									am++;
								}
								if (!found) {
									std::cout << std::endl << "Move " << toString(move) << " not part of all moves: " << fen << std::endl;
									//__debugbreak();
								};
							}
							tm++;
						}
						//check for completeness
						am = allMoves;
						while ((move = am->move)) {
							Position p3(p1);
							if (p3.GetMaterialKey() != p1.GetMaterialKey()) {
								tm = tacticalMoves;
								Move tmove = MOVE_NONE;
								bool found = false;
								while ((tmove = tm->move)) {
									if (tmove == move) {
										found = true;
										break;
									}
									tm++;
								}
								if (!found) {
									std::cout << std::endl << "Move " << toString(move) << " isn't detected as tactical: " << fen << std::endl;
									//__debugbreak();
								};
							}
							am++;
						}
						//if ((count & 1023) == 0)std::cout << std::endl << count << "\t";
						++count;
					}
					lines.clear();
				}
			}
			s.close();
			std::cout << std::endl;
			std::cout << lineCount << " lines read!" << std::endl;
		}
		else
		{
			std::cout << "Unable to open file" << std::endl;
		}
	}

	void testResult(std::string filename, Result result) {
		std::vector<std::string> lines;
		std::ifstream s;
		s.open(filename);
		std::string l;
		if (s.is_open())
		{
			unsigned long lineCount = 0;
			while (s)
			{
				std::getline(s, l);

				if ((lineCount & 32767) == 0)std::cout << ".";
				if (l.length() < 10 || !s) {
					unsigned long count = 0;
					for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it) {
						++count;
						std::string line = *it;
						const size_t indx = line.find(" c0");
						std::string fen = line.substr(0, indx);
						Position p1(fen);
						if (count < lines.size()) {
							if (p1.GetResult() != Result::OPEN) {
								std::cout << "Expected: Open Actual: " << static_cast<int>(p1.GetResult()) << "\t" << p1.fen() << std::endl;
								//__debugbreak();
								Position p2(fen);
								p2.GetResult();
							}
						}
						else {
							if (p1.GetResult() != result) {
								std::cout << "Expected: " << static_cast<int>(result) << " Actual: " << static_cast<int>(p1.GetResult()) << "\t" << p1.fen() << std::endl;
								//__debugbreak();
								Position p2(fen);
								p2.GetResult();
							}
						}
					}
					lines.clear();
				}
				else {
					lines.push_back(l);
					lineCount++;
				}
			}
			s.close();
			std::cout << std::endl;
			std::cout << lineCount << " lines read!" << std::endl;
		}
		else
		{
			std::cout << "Unable to open file" << std::endl;
		}
	}

	void testKPK() {
		Position pos("1k6/8/KP6/8/8/8/8/8 w - -0 1");
		assert(pos.evaluate() > VALUE_DRAW);
		Position pos1("1k6/8/KP6/8/8/8/8/8 b - -0 1");
		assert(pos1.evaluate() == VALUE_DRAW);
		Position pos2("8/8/8/8/8/6pk/8/6K1 w - - 0 1");
		assert(pos2.evaluate() == VALUE_DRAW);
		Position pos3("8/8/8/8/8/6pk/8/6K1 b - - 0 1");
		assert(pos3.evaluate() > VALUE_DRAW);
		std::cout << "KPK Tests succesful!" << std::endl;
	}

	bool testBBOperations()
	{
		return testPopcount() && testLSB() && testMSB();
	}

	bool testPopcount()
	{
		std::cout << "Testing Popcount..." << std::endl;
		for (int i = 0; i < 8; ++i) {
			const int pc = popcount(RANKS.at(i));
			if (pc != 8) {
				std::cout << "Error: popcount(" << RANKS.at(i) << ") = " << pc << "   should be 8" << std::endl;
				return false;
			}
		}
		for (int i = 0; i < 8; ++i) {
			const int pc = popcount(FILES.at(i));
			if (pc != 8) {
				std::cout << "Error: popcount(" << FILES.at(i) << ") = " << pc << "   should be 8" << std::endl;
				break;
			}
		}
		const std::array<Bitboard, 4> TESTS = { EMPTY, ALL_SQUARES, HALF_OF_BLACK, HALF_OF_WHITE };
		const std::array<int, 4> RESULTS = { 0, 64, 32, 32 };
		for (int i = 0; i < 4; ++i) {
			const int pc = popcount(TESTS.at(i));
			if (pc != RESULTS.at(i)) {
				std::cout << "Error: popcount(" << TESTS.at(i) << ") = " << pc << "   should be " << RESULTS.at(i) << std::endl;
				return false;
			}
		}
		Bitboard bb = ALL_SQUARES;
		int count = 64;
		do {
			const Bitboard isolated = isolateLSB(bb);
			int pc = popcount(isolated);
			if (pc != 1) {
				std::cout << "Error: popcount(" << isolated << ") = " << pc << "   should be 1" << std::endl;
				return false;
			}
			bb &= bb - 1;
			--count;
			pc = popcount(bb);
			if (pc != count) {
				std::cout << "Error: popcount(" << bb << ") = " << pc << "   should be " << count << std::endl;
				return false;
			}
		} while (bb);
		std::cout << "Popcount ok!" << std::endl;
		return true;
	}

	bool testLSB()
	{
		std::cout << "Testing LSB..." << std::endl;
		for (int i = 0; i < 8; ++i) {
			const Square pc = lsb(RANKS.at(i));
			if (pc != static_cast<Square>(A1 + 8 * i)) {
				std::cout << "Error: LSB(" << RANKS.at(i) << ") = " << pc << "   should be " << static_cast<Square>(A1 + 8 * i) << std::endl;
				return false;
			}
		}
		for (int i = 0; i < 8; ++i) {
			const Square pc = lsb(FILES.at(i));
			if (pc != static_cast<Square>(A1 + i)) {
				std::cout << "Error: LSB(" << FILES.at(i) << ") = " << pc << "   should be " << static_cast<Square>(A1 + i) << std::endl;
				return false;
			}
		}
		Bitboard bb = ALL_SQUARES;
		int count = 0;
		while (bb != EMPTY) {
			const Square sq = pop_lsb(&bb);
			if (sq != static_cast<Square>(count)) {
				std::cout << "Error: pop_lsb(" << bb << ") = " << sq << "   should be " << static_cast<Square>(count) << std::endl;
				return false;
			}
			++count;
		}
		std::cout << "LSB ok!" << std::endl;
		return true;
	}

	bool testMSB()
	{
		std::cout << "Testing MSB..." << std::endl;
		for (int i = 0; i < 8; ++i) {
			const Square pc = msb(RANKS.at(i));
			if (pc != static_cast<Square>(H1 + 8 * i)) {
				std::cout << "Error: MSB(" << RANKS.at(i) << ") = " << pc << "   should be " << static_cast<Square>(A1 + 8 * i) << std::endl;
				return false;
			}
		}
		for (int i = 0; i < 8; ++i) {
			const Square pc = msb(FILES.at(i));
			if (pc != static_cast<Square>(A8 + i)) {
				std::cout << "Error: MSB(" << FILES.at(i) << ") = " << pc << "   should be " << static_cast<Square>(A1 + i) << std::endl;
				return false;
			}
		}
		std::cout << "MSB ok!" << std::endl;
		return true;
	}

	bool testGivesCheck()
	{
		bool result = true;
		std::vector<std::string> fens = benchFens3();
		for (auto& fen : fens) {
			Position pos(fen);
			const ValuatedMove* moves = pos.GenerateMoves<LEGAL>();
			const int moveCount = pos.GeneratedMoveCount();
			for (int i = 0; i < moveCount; ++i) {
				const bool givesCheck = pos.givesCheck(moves[i].move);
				Position next(pos);
				next.ApplyMove(moves[i].move);
				result = result && (givesCheck == next.Checked());
				assert(result);
			}
		}
		return result;
	}

	bool testVerticalSymmetry()
	{
		Initialize();
		std::vector<std::string> fens(benchFens1());
		for (int i = 0; i < gsl::narrow_cast<int>(fens.size()); i++) {
			Position p(fens.at(i));
			const bool castles = p.GetCastles() != 0;
			if (castles) continue;
			std::array<ValuatedMove, 2> m;
			for (int j = 0; j < 2; ++j) {
				tt::clear();
				std::string fen = fens.at(i);
				if (j > 0) fen = utils::mirrorFenVertical(fen);
				Position pos(fen);
#pragma warning(suppress: 26409)
				Search* srch = new Search();
				srch->PrintCurrmove = false;
				srch->UciOutput = false;
				srch->NewGame();
				srch->timeManager.initialize(TimeMode::FIXED_DEPTH, 8);
				m.at(j) = srch->Think(pos);
				std::cout << toString(m.at(j).move) << " " << static_cast<int>(m.at(j).score) << "  " << fen << std::endl;
				delete srch;
			}
			std::cout << std::endl;
		}
		return true;
	}


	bool testingPlatformSpecifics()
	{
		bool resultPopcount = true;
		for (const Bitboard bb : RANKS) {
			resultPopcount = resultPopcount && popcount(bb) == 8;
			resultPopcount = resultPopcount && popcount(~bb) == 56;
		}
		for (const Bitboard bb : FILES) {
			resultPopcount = resultPopcount && popcount(bb) == 8;
			resultPopcount = resultPopcount && popcount(~bb) == 56;
		}
		resultPopcount = resultPopcount && popcount(ALL_SQUARES) == 64;
		resultPopcount = resultPopcount && popcount(EMPTY) == 0;
		if (resultPopcount) std::cout << "Popcount ok!" << std::endl; else std::cout << "Error with Popcount!" << std::endl;
		bool resultLsb = true;
		for (int i = 0; i < 8; ++i) resultLsb = resultLsb && lsb(RANKS.at(i)) == 8 * i;
		for (int i = 0; i < 8; ++i) resultLsb = resultLsb && lsb(FILES.at(i)) == i;
		resultLsb = resultLsb && lsb(ALL_SQUARES) == 0;
		if (resultLsb) std::cout << "LSB ok!" << std::endl; else std::cout << "Error with LSB!" << std::endl;
		bool resultMsb = true;
		for (int i = 0; i < 8; ++i) resultMsb = resultMsb && msb(RANKS.at(i)) == 8 * i + 7;
		for (int i = 0; i < 8; ++i) resultMsb = resultMsb && msb(FILES.at(i)) == 56 + i;
		resultMsb = resultMsb && msb(ALL_SQUARES) == 63;
		resultMsb = resultMsb && msbInt(100) == 6;
		if (resultMsb) std::cout << "MSB ok!" << std::endl; else std::cout << "Error with MSB!" << std::endl;
		return resultPopcount && resultLsb && resultMsb;
	}

	struct TBTestData {
		std::string fen;
		tablebases::WDLScore wdlScore;
		std::vector<std::string> bestMoves;
		int id;

		TBTestData(int Id, std::string fen_string, tablebases::WDLScore score, std::string move, std::string move2 = "", std::string move3 = "") {
			fen = fen_string;
			wdlScore = score;
			id = Id;
			bestMoves.push_back(move);
			if (move2.length() > 0) bestMoves.push_back(move2);
			if (move3.length() > 0) bestMoves.push_back(move3);
		}

	};

	int executeTest(TBTestData testData) {
		int result = 0;
		Position pos(testData.fen);
		if (pos.GetMaterialTableEntry()->IsTablebaseEntry()) {
			tablebases::ProbeState state;
			tablebases::RootMoves tbMoves;
			if (tablebases::probe_wdl(pos, &state) != testData.wdlScore) result = -1;
			ValuatedMove* moves = pos.GenerateMoves<LEGAL>();
			const int count = pos.GeneratedMoveCount();
			for (int i = 0; i < count; ++i) tbMoves.emplace_back(moves[i].move);
			if (!tablebases::rank_root_moves(pos, tbMoves)) result = -1;
			bool found = false;
			for (auto m : testData.bestMoves) {
				found = found || m.compare(toString(tbMoves.at(0).pv.at(0))) == 0;
			}
			if (!found) result = -1;
			if (result < 0) sync_cout << testData.id << ": Error with tablebase " << testData.fen << sync_endl;
			result = 1;
		}
		return result;
	}

	bool testTB()
	{
		std::vector<std::string> testData{ "8/8/8/8/8/8/2Rk4/1K6 b - - 0 1;-2;d2d3;;;3", "1k6/2rK4/8/8/8/8/8/8 w - - 0 1;-2;d7d6;;;3", "8/8/8/3K4/Q7/8/2p5/2k5 b - - 0 1;0;c1b1;c1b2;;4",
			"2K5/2P5/8/q7/3k4/8/8/8 w - - 0 1;0;c8b8;c8b7;;4", "4k3/4p3/8/8/8/5P2/4P1K1/8 w - - 0 1;2;f3f4;e2e3;e2e4;5", "8/4p1k1/5p2/8/8/8/4P3/4K3 b - - 0 1;2;f6f5;e7e6;e7e5;5",
			"8/4p3/p3k3/8/8/8/4P3/7K b - - 0 1;2;a6a5;;;5", "K7/8/8/k7/2Pp4/3P4/8/8 b - - 0 1;-2;a5a6;;;5", "8/8/1k2P2K/6P1/8/3p4/8/8 b - - 0 1;-1;b6c6;b6c7;;5",
			"8/8/4P2K/2k3P1/8/3p4/8/8 b - - 0 1;0;c5b4;c5d6;d3d2;5", "8/8/4R2K/2k3P1/8/3r4/8/8 b - - 0 1;-2;c5d5;d3h3;;5", "8/5r2/3R4/8/8/6k1/1P6/3K4 b - - 0 1;-2;g3g4;;;5",
			"R2K4/8/P7/2k5/8/8/8/5r2 b - - 0 1;2;f1f8;;;5", "8/q7/P6k/Q7/8/8/8/6K1 w - - 0 1;1;g1g2;;;5", "8/q7/P6k/Q7/8/8/6K1/8 b - - 0 1;-1;a7g7;;;5",
			"6k1/P7/8/Q7/8/8/2q5/K7 b - - 0 1;-2;c2c1;c2d1;;5", "8/6n1/8/7K/P7/7k/8/7N w - - 0 1;2;h5g6;;;5", "k7/n7/3P4/K5N1/8/8/8/8 w - - 0 1;1;a5a6;;;5",
			"K7/P7/k7/2n5/8/8/8/7N b - - 0 1;2;c5d7;;;5", "8/8/8/8/8/8/1nB5/kNK5 b - - 0 1;-2;b2c4;;;5", "8/8/8/8/8/8/1n5B/2K1N2k w - - 0 1;1;h2g3;;;5",
			"8/8/8/kPK5/p7/8/1P6/8 w - - 0 1;2;b5b6;;;5", "5R2/3p4/6r1/2P5/8/K7/8/6k1 w - - 0 1;2;a3b4;;;6", "8/8/8/1P3p2/8/8/2R2r2/k1K5 b - - 0 1;-1;f2f4;;;6",
			"8/8/8/1P3p2/5r2/8/2R5/k1K5 w - - 0 1;1;c2c7;;;6", "8/8/p1r5/1kp5/8/6R1/4K3/8 b - - 0 1;2;c6d6;;;6", "8/8/8/6k1/8/1P2r3/P7/R6K b - - 0 1;-2;g5g4;g5f4;;6",
			"8/8/8/8/2k5/1P2r3/P7/R1K5 b - - 0 1;2;c4c3;;;6", "8/8/8/2P5/5P2/3b4/2k4B/7K b - - 0 1;-2;d3e4;;;6", "K5b1/P7/1k6/8/4P3/8/8/1B6 b - - 0 1;2;b6c7;;;6",
			"8/8/7B/5B2/3K4/8/2k5/n6n b - - 0 1;-1;c2b3;;;6", "8/8/7B/5B2/3K4/8/2kn4/n7 b - - 0 1;0;c2d1;c2b2;;6", "NbR5/8/n7/8/8/8/8/2K2k2 w - - 0 1;1;c1d1;;;6",
			"4B3/8/5r2/8/3N1N2/8/3K4/7k w - - 0 1;2;d2e3;;;6", "n7/3p1K2/8/3N4/8/8/8/6Nk w - - 0 1;1;g1e2;g1f3;;6", "8/8/8/1kP5/6n1/Q7/7r/K7 b - - 0 1;-1;h2h1;;;6",
			"5k2/3P4/2p5/1K6/P7/8/7p/8 w - - 0 1;1;b5b6;;;6", "8/1P6/Pk6/P7/P7/8/8/K7 b - - 0 1;-2;b6a7;b6c7;;6", "6N1/5KR1/2n5/8/8/8/2n5/1k6 w - - 0 1;1;f7e6;;;6",
			"1K6/2N5/8/8/8/2N5/5kr1/6n1 b - - 0 1;1;f2e3;;;6", "6k1/8/3pP1R1/3P4/8/1K6/8/3r4 b - - 0 1;-1;g8f8;;;7", "2r5/8/8/2pP3K/2P5/3R2k1/8/8 b - - 0 1;-2;g3f4;;;7",
			"8/8/8/K7/P5R1/1k6/P1p5/r7 b - - 0 1;2;b3a3;;;7", "8/8/8/K7/P5R1/8/Pkp5/r7 w - - 0 1;0;g4c4;;;7", "8/8/8/7K/1R5P/8/5pkP/7r w - - 0 1;0;b4f4;;;7",
			"R7/pKP5/8/p5r1/k7/8/8/8 b - - 0 1;0;g5c5;;;7", "7R/5PKp/8/1r5p/7k/8/8/8 b - - 0 1;0;b5f5;;;7", "1k1K4/8/5R2/8/5P2/1p6/3P4/4r3 b - - 0 1;1;b8a7;b8b7;;7",
			"8/5k2/3p4/6P1/2B4P/8/8/1K1b4 b - - 0 1;-1;f7g6;;;7", "8/3k4/8/1B6/2p4P/8/3P4/5b1K b - - 0 1;-2;d7e6;;;7", "3K4/8/3k4/Bb6/1PP5/p7/8/8 w - - 0 1;-1;a5c7;;;7",
			"8/8/8/b7/p7/2P4K/1P6/B4k2 b - - 0 1;2;f1e2;;;7", "b4K2/1p6/2p4k/P7/B7/8/8/8 w - - 0 1;2;f8e7;;;7", "R7/1K6/4p3/p5r1/k1P5/8/8/8 w - - 0 1;0;a8c8;b7b6;;7",
			"8/8/8/2p2K2/P5R1/4P3/1k6/r7 w - - 0 1;2;g4c4;;;7", "k3B3/8/p7/P7/1P5K/8/8/6n1 w - - 0 1;1;h4g5;;;7", "8/8/6k1/6p1/4K2p/4N2P/8/3b4 b - - 0 1;2;d1e2;;;7",
			"8/b2P4/8/5Ppp/8/8/1K6/5k2 b - - 0 1;1;a7b6;;;7", "8/3P4/8/5Ppp/3b4/1K6/8/5k2 b - - 0 1;0;d4f6;;;7", "8/p6r/8/1P6/8/8/4P3/2KB3k w - - 0 1;-1;d1c2;;;7",
			"8/p7/8/1P6/7r/8/2B1P1k1/4K3 b - - 0 1;2;h4c4;h4h5;;7", "2N5/8/8/1k6/1Pp5/2P5/2b5/K7 b - - 0 1;-1;b5c6;;;7", "8/2k5/8/5N2/1Pp5/2P5/1K6/3b4 w - - 0 1;2;b2c1;;;7",
			"1N6/3pk3/1P6/2P5/8/K7/8/5n2 b - - 0 1;-1;e7d8;;;7", "8/1k1p4/1P3n2/1KP5/8/8/1N6/8 w - - 0 1;2;b2c4;;;7", "B5r1/6k1/8/8/3p3K/8/3P4/N7 w - - 0 1;1;a8d5;;;7",
			"8/2np2b1/6k1/3P4/4K3/8/8/1R6 b - - 0 1;2;c7e8;;;7", "8/8/7K/1p4P1/P7/2p5/8/4k1N1 w - - 0 1;1;g1f3;;;7", "8/8/2P5/p7/1P1Kn1p1/7k/8/8 b - - 0 1;2;e4d6;;;7",
			"k7/8/K1p5/2r5/3P4/5P2/8/5N2 b - - 0 1;1;c5c1;;;7", "1R6/8/5p2/3p4/5n2/k1P5/8/1K6 b - - 0 1;0;d5d4;;;7", "B5r1/6k1/8/8/3p3K/8/3P4/N7 w - - 0 1;1;a8d5;;;7",
			"n7/3p2b1/8/3P3k/8/8/6K1/5R2 b - - 0 1;2;h5g6;;;7", "3r4/8/2P4n/K1P5/8/1R6/8/5k2 b - - 0 1;-1;d8a8;;;7", "5K2/8/1r6/k7/2p5/2p4N/8/1R6 w - - 0 1;0;b1c1;;;7"
		};
		//5-men
		std::vector<std::vector<TBTestData>> testDataVec(5);
		int id = 0;
		for (auto s : testData) {
			auto token = utils::split(s, ';');
			const int count = std::stoi(token.at(5)) - 3;
			testDataVec.at(count).emplace_back(++id, token.at(0), static_cast<tablebases::WDLScore>(std::stoi(token.at(1))), token.at(2), token.at(3), token.at(4));
		}

		int men = 3;
		for (auto v : testDataVec) {
			sync_cout << "Testing " << men << " men" << sync_endl;
			int count = 0;
			for (auto td : v) {
				count += executeTest(td);
			}
			sync_cout << "Success Rate:" << count << " (" << std::setprecision(4) << 100.0 * count / v.size() << "%)" << sync_endl;
			++men;
		}

		return true;
	}

	bool testPack()
	{
		std::vector<std::string> testData = {
			"rnb2rk1/ppq1bppp/2pp1n2/4p3/P1PPP3/2N2N2/1P2BPPP/R1BQ1RK1 b - - 4 8 c0 7158017603000008461; c1 74380935162283148; c2 16470913450059630092; c3 132096",
			"r1b2rk1/ppq1bppp/n1pp1n2/4p3/P1PPP3/2N2N2/1P2BPPP/R1BQ1RK1 w - - 5 9 c0 14282434155469537036; c1 74380935162283320; c2 16470913450059630092; c3 148736",
			"r1b2rk1/ppq1bppp/n1pp1n2/4p3/P1PPP3/2N2N2/1P1QBPPP/R1B2RK1 b - - 6 9 c0 14282434155469537037; c1 74380935162283320; c2 16232952462858913292; c3 148992",
			"r4rk1/ppqbbppp/n1pp1n2/4p3/P1PPP3/2N2N2/1P1QBPPP/R1B2RK1 w - - 7 10 c0 14282513533865492236; c1 74380935162283320; c2 16232952462858913292; c3 165632",
			"r4rk1/ppqbbppp/n1pp1n2/4p3/P1PPP3/1PN2N2/3QBPPP/R1B2RK1 b - - 0 10 c0 14282513533865492237; c1 74380935162283320; c2 16232952462858805442; c3 163840",
			"r3r1k1/ppqbbppp/n1pp1n2/4p3/P1PPP3/1PN2N2/3QBPPP/R1B2RK1 w - - 1 11 c0 14282513533817257740; c1 74380935162283320; c2 16232952462858805442; c3 180480",
			"r3r1k1/ppqbbppp/n1pp1n2/4p3/P1PPP3/1PNQ1N2/4BPPP/R1B2RK1 b - - 2 11 c0 14282513533817257741; c1 74380935162283320; c2 16232952462783695042; c3 180736",
			"r3r1k1/ppqbbppp/2pp1n2/4p3/PnPPP3/1PNQ1N2/4BPPP/R1B2RK1 w - - 3 12 c0 14282513533817257740; c1 74421617323052104; c2 16232952462783695042; c3 197376",
			"r3r1k1/ppqbbppp/2pp1n2/4p3/PnPPP3/1PN2N2/4BPPP/R1BQ1RK1 b - - 4 12 c0 14282513533817257741; c1 74421617323052104; c2 16470913450059522242; c3 197632",
			"3rr1k1/ppqbbppp/2pp1n2/4p3/PnPPP3/1PN2N2/4BPPP/R1BQ1RK1 w - - 5 13 c0 14282513533818576652; c1 74421617323052104; c2 16470913450059522242; c3 214272",
			"3rr1k1/ppqbbppp/2pp1n2/4p3/PnPPP3/BPN2N2/4BPPP/R2Q1RK1 b - - 6 13 c0 14282513533818576653; c1 74421617323052104; c2 16467139643406945317; c3 214528",
			"3rr1k1/1pqbbppp/2pp1n2/p3p3/PnPPP3/BPN2N2/4BPPP/R2Q1RK1 w - - 0 14 c0 10116029132479864588; c1 74421617338509508; c2 16467139643406945317; c3 229376",
			"3rr1k1/1pqbbppp/2pp1n2/p3p3/PnPPP3/1PN2N2/1B2BPPP/R2Q1RK1 b - - 1 14 c0 10116029132479864589; c1 74421617338509508; c2 16467139643417059522; c3 229632",
			"3rr1k1/1pqbbpp1/2pp1n1p/p3p3/PnPPP3/1PN2N2/1B2BPPP/R2Q1RK1 w - - 0 15 c0 4927882361749053196; c1 74421617338918540; c2 16467139643417059522; c3 245760",
			"3rr1k1/1pqbbpp1/2pp1n1p/p2Pp3/PnP1P3/1PN2N2/1B2BPPP/R2Q1RK1 b - - 0 15 c0 4927882361749053197; c1 73853169693140620; c2 16467139643417059522; c3 245760",
			"3r1rk1/1pqbbpp1/2pp1n1p/p2Pp3/PnP1P3/1PN2N2/1B2BPPP/R2Q1RK1 w - - 1 16 c0 4927882361797287692; c1 73853169693140620; c2 16467139643417059522; c3 262400",
			"3r1rk1/1pqbbpp1/2pp1n1p/p2Pp3/PnP1P3/1PN5/1B1NBPPP/R2Q1RK1 b - - 2 16 c0 4927882361797287693; c1 73853169693140620; c2 16467139643432370370; c3 262656",
			"3r1r2/1pqbbppk/2pp1n1p/p2Pp3/PnP1P3/1PN5/1B1NBPPP/R2Q1RK1 w - - 3 17 c0 5244078705069202316; c1 73853169693140620; c2 16467139643432370370; c3 279296",
			"3r1r2/1pqbbppk/2pp1n1p/p2Pp3/PnP1P3/1PN5/1B1NBPPP/R1Q2RK1 b - - 4 17 c0 5244078705069202317; c1 73853169693140620; c2 16305010056847032514; c3 279552",
			"4rr2/1pqbbppk/2pp1n1p/p2Pp3/PnP1P3/1PN5/1B1NBPPP/R1Q2RK1 w - - 5 18 c0 5244078705070709644; c1 73853169693140620; c2 16305010056847032514; c3 296192",
			"4rr2/1pqbbppk/2pp1n1p/p2Pp3/PnP1P3/1PN5/1B1NBPPP/RQ3RK1 b - - 6 18 c0 5244078705070709645; c1 73853169693140620; c2 16223945263554363586; c3 296448",
			"4rr1k/1pqbbpp1/2pp1n1p/p2Pp3/PnP1P3/1PN5/1B1NBPPP/RQ3RK1 w - - 7 19 c0 4927882361798795148; c1 73853169693140620; c2 16223945263554363586; c3 313088",
			"4rr1k/1pqbbpp1/2pp1n1p/p2Pp3/PnP1P3/1PN4P/1B1NBPP1/RQ3RK1 b - - 0 19 c0 4927882361798795149; c1 73853169693140620; c2 16223940739998974146; c3 311296",
			"4rr2/1pqbbppk/2pp1n1p/p2Pp3/PnP1P3/1PN4P/1B1NBPP1/RQ3RK1 w - - 1 20 c0 5244078705070709644; c1 73853169693140620; c2 16223940739998974146; c3 327936",
			"4rr2/1pqbbppk/2pp1n1p/p2Pp3/PnP1P3/1P5P/NB1NBPP1/RQ3RK1 b - - 2 20 c0 5244078705070709645; c1 73853169693140620; c2 16223940739999139842; c3 328192",
			"4rr2/1pqbbppk/n1pp1n1p/p2Pp3/P1P1P3/1P5P/NB1NBPP1/RQ3RK1 w - - 3 21 c0 4091157200463862668; c1 73202256803686597; c2 16223940739999139842; c3 344832",
			"4rr2/1pqbbppk/n1pp1n1p/p2Pp3/P1P1P3/1P5P/NBQNBPP1/R4RK1 b - - 4 21 c0 4091157200463862669; c1 73202256803686597; c2 16172499180309383170; c3 345088",
			"4r1r1/1pqbbppk/n1pp1n1p/p2Pp3/P1P1P3/1P5P/NBQNBPP1/R4RK1 w - - 5 22 c0 4091157200560331660; c1 73202256803686597; c2 16172499180309383170; c3 361728",
			"4r1r1/1pqbbppk/n1pp1n1p/p2Pp3/P1P1P3/1P5P/NBQNBPP1/1R3RK1 b - - 6 22 c0 4091157200560331661; c1 73202256803686597; c2 16204024377700976642; c3 361984",
			"4r1r1/1p1bbppk/nqpp1n1p/p2Pp3/P1P1P3/1P5P/NBQNBPP1/1R3RK1 w - - 7 23 c0 3714461802464942988; c1 73202256803686599; c2 16204024377700976642; c3 378624",
			"4r1r1/1p1bbppk/nqpp1n1p/p2Pp3/P1P1P3/1P5P/NBQNBPP1/4RRK1 b - - 8 23 c0 3714461802464942989; c1 73202256803686599; c2 16645377141183285250; c3 378880",
			"1r4r1/1p1bbppk/nqpp1n1p/p2Pp3/P1P1P3/1P5P/NBQNBPP1/4RRK1 w - - 9 24 c0 3714461802462305164; c1 73202256803686599; c2 16645377141183285250; c3 395520",
			"1r4r1/1p1bbppk/nqpp1n1p/p2Pp3/P1P1P3/1PN4P/1BQNBPP1/4RRK1 b - - 10 24 c0 3714461802462305165; c1 73202256803686599; c2 16645377141183119554; c3 395776",
			"1r4r1/1p1bbppk/1qpp1n1p/p1nPp3/P1P1P3/1PN4P/1BQNBPP1/4RRK1 w - - 11 25 c0 8254090226851765132; c1 73202257802864268; c2 16645377141183119554; c3 412416",
			"1r4r1/1p1bbppk/1qpp1n1p/p1nPp3/P1P1P3/BPN4P/2QNBPP1/4RRK1 b - - 12 25 c0 8254090226851765133; c1 73202257802864268; c2 16645377141173128229; c3 412672",
			"1r2r3/1p1bbppk/1qpp1n1p/p1nPp3/P1P1P3/BPN4P/2QNBPP1/4RRK1 w - - 13 26 c0 8254090226707061644; c1 73202257802864268; c2 16645377141173128229; c3 429312",
			"1r2r3/1p1bbppk/1qpp1n1p/p1nPp3/P1P1P3/BPN4P/2QNBPP1/R4RK1 b - - 14 26 c0 8254090226707061645; c1 73202257802864268; c2 16172499180299226149; c3 429568",
			"r3r3/1p1bbppk/1qpp1n1p/p1nPp3/P1P1P3/BPN4P/2QNBPP1/R4RK1 w - - 15 27 c0 8254090226706873228; c1 73202257802864268; c2 16172499180299226149; c3 446208",
			"r3r3/1p1bbppk/1qpp1n1p/p1nPp3/P1P1P3/BPN4P/3NBPP1/R1Q2RK1 b - - 16 27 c0 8254090226706873229; c1 73202257802864268; c2 16305005533281651749; c3 446464",
			"2r1r3/1p1bbppk/1qpp1n1p/p1nPp3/P1P1P3/BPN4P/3NBPP1/R1Q2RK1 w - - 17 28 c0 8254090226707438476; c1 73202257802864268; c2 16305005533281651749; c3 463104",
			"2r1r3/1p1bbppk/1qpp1n1p/p1BPp3/P1P1P3/1PN4P/3NBPP1/R1Q2RK1 b - - 0 28 c0 8254090226707438477; c1 73202256863340172; c2 1019062845830103234; c3 28672",
			"2r1r3/1p1bbppk/2pp1n1p/p1qPp3/P1P1P3/1PN4P/3NBPP1/R1Q2RK1 w - - 0 29 c0 14162812937817529228; c1 2310418150351538792; c2 63691427864381452; c3 1856",
			"2r1r3/1p1bbppk/2pp1n1p/p1qPp3/P1P1P3/1PN4P/2QNBPP1/R4RK1 b - - 1 29 c0 14162812937817529229; c1 2310418150351538792; c2 63173824923043852; c3 1857",
			"2r1r3/1p1bbp1k/2pp1npp/p1qPp3/P1P1P3/1PN4P/2QNBPP1/R4RK1 w - - 0 30 c0 10107321473370397580; c1 2310418150351539302; c2 63173824923043852; c3 1920",
			"2r1r3/1p1bbp1k/2pp1npp/p1qPp3/P1P1P3/1PN4P/2QNBPP1/1R3RK1 b - - 1 30 c0 10107321473370397581; c1 2310418150351539302; c2 63296970225354764; c3 1921",
			"2r1r3/1p1bbpk1/2pp1npp/p1qPp3/P1P1P3/1PN4P/2QNBPP1/1R3RK1 w - - 2 31 c0 10107321473370397452; c1 2310418150351539302; c2 63296970225354764; c3 1986",
			"2r1r3/1p1bbpk1/2pp1npp/p1qPp3/P1P1P3/1PN4P/2QNBPP1/4RRK1 b - - 3 31 c0 10107321473370397453; c1 2310418150351539302; c2 65021004457707532; c3 1987",
			"2r1r1k1/1p1bbp2/2pp1npp/p1qPp3/P1P1P3/1PN4P/2QNBPP1/4RRK1 w - - 4 32 c0 10106699518619393804; c1 2310418150351539302; c2 65021004457707532; c3 2052",
			"2r1r1k1/1p1bbp2/2pp1npp/p1qPp3/P1P1P3/1PN4P/2QNBPP1/R4RK1 b - - 5 32 c0 10106699518619393805; c1 2310418150351539302; c2 63173824923043852; c3 2053",
			"2r1r3/1p1bbpk1/2pp1npp/p1qPp3/P1P1P3/1PN4P/2QNBPP1/R4RK1 w - - 6 33 c0 10107321473370397452; c1 2310418150351539302; c2 63173824923043852; c3 2118",
			"2r1r3/1p1bbpk1/2pp1npp/p1qPp3/P1P1P3/1PN4P/2QNBPP1/1R3RK1 b - - 7 33 c0 10107321473370397453; c1 2310418150351539302; c2 63296970225354764; c3 2119",
			"2r4r/1p1bbpk1/2pp1npp/p1qPp3/P1P1P3/1PN4P/2QNBPP1/1R3RK1 w - - 8 34 c0 10107321473708038924; c1 2310418150351539302; c2 63296970225354764; c3 2184",
			"2r4r/1p1bbpk1/2pp1npp/p1qPp3/P1P1P3/1PN4P/2QNBPP1/1RR3K1 b - - 9 34 c0 10107321473708038925; c1 2310418150351539302; c2 8127874790066188; c3 2185",
			"2r4r/1p1bbpkn/2pp2pp/p1qPp3/P1P1P3/1PN4P/2QNBPP1/1RR3K1 w - - 10 35 c0 14167316537782541068; c1 2310418150351539272; c2 8127874790066188; c3 2250",
			"2r4r/1p1bbpkn/2pp2pp/p1qPp3/P1P1P3/1PN4P/2Q1BPP1/1RR2NK1 b - - 11 35 c0 14167316537782541069; c1 2310418150351539272; c2 27529589939243020; c3 2251",
			"2r4r/1p1b1pkn/2pp2pp/p1qPp1b1/P1P1P3/1PN4P/2Q1BPP1/1RR2NK1 w - - 12 36 c0 10108827804638092044; c1 2310418442452539588; c2 27529589939243020; c3 2316",
			"2r4r/1p1b1pkn/2pp2pp/p1qPp1b1/P1P1P3/1PN4P/2Q1BPP1/1R2RNK1 b - - 13 36 c0 10108827804638092045; c1 2310418442452539588; c2 29007333566973964; c3 2317",
			"2r1r3/1p1b1pkn/2pp2pp/p1qPp1b1/P1P1P3/1PN4P/2Q1BPP1/1R2RNK1 w - - 14 37 c0 10108827804300450572; c1 2310418442452539588; c2 29007333566973964; c3 2382",
			"2r1r3/1p1b1pkn/2pp2pp/p1qPp1b1/P1P1P3/1PN4P/2Q1BPP1/R3RNK1 b - - 15 37 c0 10108827804300450573; c1 2310418442452539588; c2 28999636985579532; c3 2383",
			"2r1r3/1p1b1pk1/2pp1npp/p1qPp1b1/P1P1P3/1PN4P/2Q1BPP1/R3RNK1 w - - 16 38 c0 7549235103582100236; c1 2310418442452539590; c2 28999636985579532; c3 2448",
			"2r1r3/1p1b1pk1/2pp1npp/p1qPp1b1/P1P1P3/1PN2B1P/2Q2PP1/R3RNK1 b - - 17 38 c0 7549235103582100237; c1 2310418442452539590; c2 28999636937116172; c3 2449",
			"2r1r3/1p1b1pk1/3p1npp/p1qpp1b1/P1P1P3/1PN2B1P/2Q2PP1/R3RNK1 w - - 0 39 c0 7388231416903605004; c1 13979459207943836044; c2 1812477308569760; c3 156",
			"2r1r3/1p1b1pk1/3p1npp/p1qPp1b1/P1P5/1PN2B1P/2Q2PP1/R3RNK1 b - - 0 39 c0 7388231416903605005; c1 873702817310593420; c2 13835171335113949322; c3 9",
			"2r1r3/1p3pk1/3p1npp/p1qPpbb1/P1P5/1PN2B1P/2Q2PP1/R3RNK1 w - - 1 40 c0 14296822445803019020; c1 873702817746969880; c2 72170873869713546; c3 10",
			"2r1r3/1p3pk1/3p1npp/p1qPpbb1/P1P1N3/1P3B1P/2Q2PP1/R3RNK1 b - - 2 40 c0 14296822445803019021; c1 144964103043081496; c2 144228467907641482; c3 10",
			"2r1r3/1p3pk1/3p1npp/p2Ppbb1/PqP1N3/1P3B1P/2Q2PP1/R3RNK1 w - - 3 41 c0 14296822445803019020; c1 144967452775745816; c2 4827972080372957322; c3 10",
			"2r1r3/1p3pk1/3p1npp/p2Ppbb1/PqP1N3/1P3BNP/2Q2PP1/R3R1K1 b - - 4 41 c0 14296822445803019021; c1 144967452775745816; c2 4900040022027405514; c3 10",
			"2r1r3/1p3pk1/3p1npp/p2Pp1b1/PqP1b3/1P3BNP/2Q2PP1/R3R1K1 w - - 0 42 c0 14296822445803019020; c1 11538592171214047512; c2 12105683525087412364; c3 0",
			"2r1r3/1p3pk1/3p1npp/p2Pp1b1/PqP1N3/1P3B1P/2Q2PP1/R3R1K1 b - - 0 42 c0 14296822445803019021; c1 11538275511865248024; c2 756605220317963272; c3 0",
			"2r1r3/1p3pk1/3p2pp/p2Pp1b1/PqP1n3/1P3B1P/2Q2PP1/R3R1K1 w - - 0 43 c0 10108474792348457740; c1 9944531848532398225; c2 48413726176715328; c3 0",
			"2r1r3/1p3pk1/3p2pp/p2Pp1b1/PqP1B3/1P5P/2Q2PP1/R3R1K1 b - - 0 43 c0 10108474792348457741; c1 577029230438977681; c2 3025857886044708; c3 0",
			"2r1r3/1p3pk1/3p2pp/p2Pp3/PqP1Bb2/1P5P/2Q2PP1/R3R1K1 w - - 1 44 c0 10108474792348457740; c1 577070227256316049; c2 3097326141850148; c3 0",
			"2r1r3/1p3pk1/3p2pp/p2Pp3/PqP1Bb2/1P4PP/2Q2P2/R3R1K1 b - - 0 44 c0 10108474792348457741; c1 9512211887959380113; c2 3096226630214208; c3 0",
			"2r1r3/1p3pk1/3p2pp/p2Pp1b1/PqP1B3/1P4PP/2Q2P2/R3R1K1 w - - 1 45 c0 10108474792348457740; c1 9512170891142041745; c2 3167694886019648; c3 0",
			"2r1r3/1p3pk1/3p2pp/p2Pp1b1/PqP1B2P/1P4P1/2Q2P2/R3R1K1 b - - 0 45 c0 10108474792348457741; c1 4620839483352617105; c2 3166595374391872; c3 0",
			"2r1r3/1p3pk1/3p2pp/p2Pp3/PqP1B2P/1P4P1/2Qb1P2/R3R1K1 w - - 1 46 c0 10108474792348457740; c1 288802467709259921; c2 3238063630199460; c3 0",
			"2r1r3/1p3pk1/3p2pp/p2Pp3/PqP1B2P/1P4P1/2Qb1P2/R2R2K1 b - - 2 46 c0 10108474792348457741; c1 288802467709259921; c2 3239162202303140; c3 0",
			"2r1r3/1p3pk1/3p2pp/p2Pp3/PqP1B2P/1Pb3P1/2Q2P2/R2R2K1 w - - 3 47 c0 10108474792348457740; c1 4990560478684057745; c2 3310630458106432; c3 0",
			"2r1r3/1p3pk1/3p2pp/p2Pp3/PqP1B2P/1Pb3P1/2Q2P2/1R1R2K1 b - - 4 47 c0 10108474792348457741; c1 4990560478684057745; c2 3311729977074240; c3 0",
			"2r1r3/1p4k1/3p2pp/p2Ppp2/PqP1B2P/1Pb3P1/2Q2P2/1R1R2K1 w - - 0 48 c0 1784700904229542668; c1 4990560478684131593; c2 3377700674740800; c3 0",
			"2r1r3/1p4k1/3p2pp/p2Ppp2/PqP4P/1Pb3P1/2Q2P2/1R1R2KB b - - 1 48 c0 1784700904229542669; c1 311910010237104393; c2 3378804660503076; c3 0",
			"2r1r3/1p4k1/3p2pp/p2P1p2/PqP1p2P/1Pb3P1/2Q2P2/1R1R2KB w - - 0 49 c0 1784700904229542668; c1 311910081457365513; c2 3448073893052964; c3 0",
			"2r1r3/1p4k1/3p2pp/p2P1p2/PqP1p2P/1Pb3P1/2Q2P2/2RR2KB b - - 1 49 c0 1784700904229542669; c1 311910081457365513; c2 3449173405598244; c3 0",
			"2r1r3/1p4k1/3p1bpp/p2P1p2/PqP1p2P/1P4P1/2Q2P2/2RR2KB w - - 2 50 c0 10118049064725224204; c1 288803292343050385; c2 3520641661403684; c3 0",
			"2r1r3/1p4k1/3p1bpp/p2P1p2/PqP1p2P/1P4P1/2Q2P2/2R1R1KB b - - 3 50 c0 10118049064725224205; c1 288803292343050385; c2 3521741231751716; c3 0",
			"2r1r3/1p4k1/3p2pp/p2P1p2/PqPbp2P/1P4P1/2Q2P2/2R1R1KB w - - 4 51 c0 1784700904229542668; c1 288803335646028297; c2 3593209487557156; c3 0",
			"2r1r3/1p4k1/3p2pp/p2P1p1P/PqPbp3/1P4P1/2Q2P2/2R1R1KB b - - 0 51 c0 1784700904229542669; c1 288812741271003657; c2 3588811441046052; c3 0",
			"2r1r3/1p4k1/3p2pp/p2P1p1P/PqPb4/1P2p1P1/2Q2P2/2R1R1KB w - - 0 52 c0 1784700904229542668; c1 364827477657297417; c2 3659180185223716; c3 0",
			"2r1r3/1p4k1/3p2pp/p2P1p1P/PqPb4/1P2p1P1/2Q2P1K/2R1R2B b - - 1 52 c0 1784700904229542687; c1 364827477657297417; c2 3660279637213732; c3 0",
			"2r1r3/1p4k1/3p2pp/p2P1p1P/PqPb4/1P4P1/2Q2p1K/2R1R2B w - - 0 53 c0 1784700904229542686; c1 4629736324777157129; c2 233096804360738; c3 0",
			"2r1R3/1p4k1/3p2pp/p2P1p1P/PqPb4/1P4P1/2Q2p1K/2R4B b - - 0 53 c0 1784700904195988255; c1 4629736324777157129; c2 14568550097442; c3 0",
			"4r3/1p4k1/3p2pp/p2P1p1P/PqPb4/1P4P1/2Q2p1K/2R4B w - - 0 54 c0 10487837347975928606; c1 2595201529512266272; c2 927714250274; c3 0",
			"4r3/1p4k1/3p2pp/p2P1p1P/PqPb4/1P4P1/2Q2p1K/5R1B b - - 1 54 c0 10487837347975928607; c1 2595201529512266272; c2 927982710818; c3 0",
			"8/1p4k1/3p2pp/p2P1p1P/PqPb4/1P4P1/2Q2p1K/4rR1B w - - 2 55 c0 655489834248313630; c1 2468043104808210594; c2 945431018370; c3 0",
			"8/1p4k1/3p2pp/p2P1p1P/PqPb4/1P4P1/2Q2pBK/4rR2 b - - 3 55 c0 655489834248313631; c1 2468043104808210594; c2 945698617366; c3 0",
			"8/1p4k1/3p2pp/p2P1p1P/PqPb4/1P2r1P1/2Q2pBK/5R2 w - - 4 56 c0 655489834248313630; c1 2595604045816271010; c2 963146875234; c3 0",
			"8/1p4k1/3p2pp/p2P1p1P/PqPb4/1P2r1P1/5QBK/5R2 b - - 0 56 c0 655489834248313631; c1 2307373669664559266; c2 60129570837; c3 0",
			"8/1p4k1/3p2pp/p2P1p1P/PqPb4/1P1r2P1/5QBK/5R2 w - - 1 57 c0 655489834248313630; c1 2307171359525048482; c2 61220089877; c3 0",
			"8/1p4k1/3p2pp/p2P1p1P/PqPb4/1P1r2P1/6BK/4QR2 b - - 2 57 c0 655489834248313631; c1 4613014368738742434; c2 61236868225; c3 0",
			"8/1p4k1/3p2pp/p2P1p1P/P1Pb4/1P1r2P1/6BK/4qR2 w - - 0 58 c0 655489834248313630; c1 1441234902652817570; c2 3892316104; c3 0",
			"8/1p4k1/3p2pp/p2P1p1P/P1Pb4/1P1r2P1/6BK/4R3 b - - 0 58 c0 655489834248313631; c1 1441234902652817570; c2 243269688; c3 0",
			"8/1p4k1/3p2pp/p2Pbp1P/P1P5/1P1r2P1/6BK/4R3 w - - 1 59 c0 655489834248313630; c1 1441234902480652853; c2 247529528; c3 0",
			"8/1p4k1/3p2pp/p2Pbp1P/P1P5/1P1r2P1/6BK/1R6 b - - 2 59 c0 655489834248313631; c1 1441234902480652853; c2 247595015; c3 0",
			"8/1p4k1/3p2pp/p2Pbp1P/P1P5/1P4r1/6BK/1R6 w - - 0 60 c0 655489834248313630; c1 8160623688459291189; c2 15728640; c3 0",
			"8/1p4k1/3p2Pp/p2Pbp2/P1P5/1P4r1/6BK/1R6 b - - 0 60 c0 655419465504135967; c1 510038980528706101; c2 983040; c3 0",
			"8/1p4k1/3p2Pp/p2Pbp2/P1P5/1r6/6BK/1R6 w - - 0 61 c0 655419465504135966; c1 31877053460779573; c2 62464; c3 0",
			"8/1p4k1/3p2Pp/p2Pbp2/P1P5/1r6/6B1/1R4K1 b - - 1 61 c0 655419465504135949; c1 63402250852373045; c2 62480; c3 0",
			"8/1p4k1/3p2Pp/p2Pbp2/P1P5/8/6B1/1r4K1 w - - 0 62 c0 655419465504135948; c1 12969839161512501; c2 3968; c3 0",
			"8/1p4k1/3p2Pp/p2Pbp2/P1P5/8/5KB1/1r6 b - - 1 62 c0 655419465504135963; c1 6484919580889653; c2 3969; c3 0",
			"8/1p6/3p2kp/p2Pbp2/P1P5/8/5KB1/1r6 w - - 0 63 c0 5805572096541071130; c1 405307473805603; c2 252; c3 0",
			"8/1p6/3p2kp/p2Pbp2/P1P5/5B2/5K2/1r6 b - - 1 63 c0 5805572096541071131; c1 1153326127570239779; c2 252; c3 0",
			"8/1p6/3p2kp/p2Pbp2/P1P5/5B2/5K2/r7 w - - 2 64 c0 5805572096541071130; c1 2306045322037575971; c2 256; c3 0",
			"8/1p6/3p2kp/p2Pbp2/P1P5/8/4BK2/r7 b - - 3 64 c0 5805572096541071131; c1 3458967167557452067; c2 256; c3 0",
			"8/1p6/3p2kp/p2Pbp2/r1P5/8/4BK2/8 w - - 0 65 c0 5805572096541071130; c1 4611686362024793891; c2 16; c3 0",
			"8/1p6/3p2kp/p2Pbp2/r1P5/8/4B3/4K3 b - - 1 65 c0 5805572096541071113; c1 4683743956062721827; c2 16; c3 0",
			"8/1p6/3p2kp/p2Pbp2/2P5/8/r3B3/4K3 w - - 2 66 c0 5805572096541071112; c1 9367487574702031907; c2 16; c3 0",
			"8/1p6/3p2kp/p2Pbp2/2P5/3B4/r7/4K3 b - - 3 66 c0 5805572096541071113; c1 9439544917794751523; c2 16; c3 0",
			"8/1p6/3p2kp/p2P1p2/2P5/3B2b1/r7/4K3 w - - 4 67 c0 2346807582720530184; c1 14123288531630030914; c2 16; c3 0",
			"8/1p6/3p2kp/p2P1p2/2P5/3B2b1/r7/3K4 b - - 5 67 c0 2346807582720530183; c1 14195346125667958850; c2 16; c3 0",
			"8/1p6/3p2kp/3P1p2/p1P5/3B2b1/r7/3K4 w - - 0 68 c0 2452360698987026182; c1 100196155473; c2 17; c3 0",
			"8/1p6/3p2kp/2PP1p2/p7/3B2b1/r7/3K4 b - - 0 68 c0 2343148408023291655; c1 100196155666; c2 17; c3 0",
			"8/1p6/6kp/2pP1p2/p7/3B2b1/r7/3K4 w - - 0 69 c0 2453415611674400518; c1 1441151887020818449; c2 1; c3 0",
			"8/1p6/6kp/1BpP1p2/p7/6b1/r7/3K4 b - - 1 69 c0 2361197372429571847; c1 1445655486648025362; c2 1; c3 0",
			"8/1p6/7p/1BpP1pk1/p7/6b1/r7/3K4 w - - 2 70 c0 4722394744787833606; c1 1738389462427107604; c2 1; c3 0",
			"8/1p6/7p/1BpP1pk1/p7/6b1/r7/2K5 b - - 3 70 c0 4722394744787833605; c1 1742893062054478100; c2 1; c3 0",
			"8/1p6/7p/1BpP1pk1/8/p5b1/r7/2K5 w - - 0 71 c0 4722394744787833604; c1 2017612639324147716; c2 1; c3 0",
			"4B3/1p6/7p/2pP1pk1/8/p5b1/r7/2K5 b - - 1 71 c0 4722323277602231045; c1 2022116238951518212; c2 1; c3 0",
			"4B3/1p6/5k1p/2pP1p2/8/p5b1/r7/2K5 w - - 2 72 c0 2361161639371871876; c1 2314850214730600450; c2 1; c3 0",
			"4B3/1p6/5k1p/2pP1p2/8/p5b1/r7/3K4 b - - 3 72 c0 2361161639371871879; c1 2319353814357970946; c2 1; c3 0",
			"4B3/1p6/5k1p/2pP1p2/8/p5b1/5r2/3K4 w - - 4 73 c0 2361161639371871878; c1 2612087981531533314; c2 1; c3 0",
			"8/1p6/5k1p/1BpP1p2/8/p5b1/5r2/3K4 b - - 5 73 c0 2361197372429571719; c1 2616591581158903810; c2 1; c3 0",
			"8/1p6/5k1p/1BpP1p2/8/6b1/p4r2/3K4 w - - 0 74 c0 2361197372429571718; c1 2882303959376330754; c2 1; c3 0",
			"8/1p6/5k1p/2pP1p2/8/6b1/p3Br2/3K4 b - - 1 74 c0 2453415611674400391; c1 2886807560073330688; c2 1; c3 0",
			"8/1p6/5k1p/2pP1p2/8/6b1/4Br2/q2K4 w - - 0 75 c0 2453415611674400390; c1 3170535868088008704; c2 1; c3 0",
			"8/1p6/5k1p/2pP1p2/8/6b1/2K1Br2/q7 b - - 1 75 c0 2453415611674400405; c1 3175038602505961472; c2 1; c3 0",
			"8/1p6/5k1p/2pP1p2/8/6b1/q1K1Br2/8 w - - 2 76 c0 2453415611674400404; c1 3467771812557176832; c2 1; c3 0",
			"8/1p6/5k1p/2pP1p2/8/3K2b1/q3Br2/8 b - - 3 76 c0 2453415611674400423; c1 3472275412171268096; c2 1; c3 0",
			"rn1qkb1r/ppp2p1p/6p1/3bp3/3Pn3/3B4/PPP2PPP/R1BQK1NR b KQkq - 1 8 c0 11693088160556711433; c1 1164201429052179464; c2 1701345347944849474; c3 32",
			"rn1qkb1r/ppp2p1p/2b3p1/4p3/3Pn3/3B4/PPP2PPP/R1BQK1NR w KQkq - 2 9 c0 11693088160556711432; c1 1164201429041431880; c2 2854266852551696450; c3 36",
			"rn1qkb1r/ppp2p1p/2b3p1/3Pp3/4n3/3B4/PPP2PPP/R1BQK1NR b KQkq - 0 9 c0 11693088160556711433; c1 1164201398968272200; c2 548423843338002498; c3 36",
			"rn2kb1r/ppp2p1p/2b3p1/3qp3/4n3/3B4/PPP2PPP/R1BQK1NR w KQkq - 0 10 c0 9954190046868209160; c1 2378605596661793876; c2 9257648527063400964; c3 2",
			"rn2kb1r/ppp2p1p/2b3p1/3qp3/4n3/3BB3/PPP2PPP/R2QK1NR b KQkq - 1 10 c0 9954190046868209161; c1 1176144496153871444; c2 9329706117502345282; c3 2",
			"r3kb1r/pppn1p1p/2b3p1/3qp3/4n3/3BB3/PPP2PPP/R2QK1NR w KQkq - 2 11 c0 9955706368166133256; c1 1176144496153871444; c2 14013449729967661122; c3 2",
			"r3kb1r/pppn1p1p/2b3p1/3qp3/4n3/P2BB3/1PP2PPP/R2QK1NR b KQkq - 0 11 c0 9955706368166133257; c1 371565867412505684; c2 13869334541891805250; c3 2",
			"r3kb1r/pppn1p1p/6p1/3qp3/b3n3/P2BB3/1PP2PPP/R2QK1NR w KQkq - 1 12 c0 9955706368166133256; c1 371565872769958976; c2 106334080647569474; c3 3",
			"r3kb1r/pppn1p1p/6p1/3qp3/b3n3/P2BBN2/1PP2PPP/R2QK2R b KQkq - 2 12 c0 9955706368166133257; c1 7289094900411040832; c2 178389744496018464; c3 3",
			"r3kb1r/pppn3p/6p1/3qpp2/b3n3/P2BBN2/1PP2PPP/R2QK2R w KQkq - 0 13 c0 615240740999724552; c1 7289094900429211716; c2 4645960574847550496; c3 3",
			"r3kb1r/pppn3p/6p1/3qpp2/b1P1n3/P2BBN2/1P3PPP/R2QK2R b KQkq - 0 13 c0 615240740999724553; c1 5945053896680523844; c2 4645960574847549958; c3 3",
			"r3kb1r/pppn3p/6p1/q3pp2/b1P1n3/P2BBN2/1P3PPP/R2QK2R w KQkq - 1 14 c0 615240740999724552; c1 5945053896680479044; c2 9329704187312865798; c3 3",
			"r3kb1r/pppn3p/6p1/q3pp2/bPP1n3/P2BBN2/5PPP/R2QK2R b KQkq - 0 14 c0 615240740999724553; c1 2887141901821090116; c2 9257646593274937445; c3 3",
			"r3kb1r/pppn3p/6p1/q3pp2/1PP1n3/P2BBN2/5PPP/R2bK2R w KQkq - 0 15 c0 615240740999724552; c1 5945053891580205380; c2 4325598214368796678; c3 0",
			"r3kb1r/pppn3p/6p1/P3pp2/2P1n3/P2BBN2/5PPP/R2bK2R b KQkq - 0 15 c0 615240740999724553; c1 7289094895865758020; c2 270349888398049792; c3 0",
			"r3kb1r/pppn3p/6p1/P3pp2/b1P1n3/P2BBN2/5PPP/R3K2R w KQkq - 1 16 c0 615240740999724552; c1 5945053896680472900; c2 288645718556811270; c3 0",
			"r3kb1r/pppn3p/6p1/P3pp2/b1P1n3/P2BBN2/5PPP/1R2K2R b Kkq - 2 16 c0 615240740999724553; c1 5945053896680472900; c2 288909603226525702; c3 0",
			"r3kb1r/pppn3p/6p1/P1n1pp2/b1P5/P2BBN2/5PPP/1R2K2R w Kkq - 3 17 c0 615240740999724552; c1 5945033348766548292; c2 307205476712718342; c3 0",
			"r3kb1r/pppn3p/6p1/P1n1pp2/b1P5/P3BN2/4BPPP/1R2K2R b Kkq - 4 17 c0 615240740999724553; c1 7278098838468215108; c2 307486951689430272; c3 0",
			"2kr1b1r/pppn3p/6p1/P1n1pp2/b1P5/P3BN2/4BPPP/1R2K2R w K - 5 18 c0 615240741000289544; c1 7278098838468215108; c2 325677272059356416; c3 0",
			"2kr1b1r/pppn3p/6p1/P1n1pp2/b1P5/P3BN2/4BPPP/1R3RK1 b - - 6 18 c0 615240741000289549; c1 7278098838468215108; c2 325948988870370560; c3 0",
			"2kr3r/pppn3p/3b2p1/P1n1pp2/b1P5/P3BN2/4BPPP/1R3RK1 w - - 7 19 c0 12144128344682372364; c1 7278098838468215108; c2 344244862356563200; c3 0",
			"2kr3r/pppn3p/3b2p1/P1n1pp2/b1P5/P3BNP1/4BP1P/1R3RK1 b - - 0 19 c0 12144128344682372365; c1 7278098838468215108; c2 342274537519468548; c3 0",
			"2kr3r/pppn3p/3b2p1/P1n1p3/b1P2p2/P3BNP1/4BP1P/1R3RK1 w - - 0 20 c0 12144128344682372364; c1 7278135879037927748; c2 360288936028950532; c3 0",
			"2kr3r/pppn3p/3b2p1/P1n1p3/b1P2P2/P3BN2/4BP1P/1R3RK1 b - - 0 20 c0 12144128344682372365; c1 7278100694665838916; c2 22518058501809408; c3 0",
			"2krr3/pppn3p/3b2p1/P1n1p3/b1P2P2/P3BN2/4BP1P/1R3RK1 w - - 1 21 c0 12144128344513551628; c1 7278100694665838916; c2 23661550594696448; c3 0",
			"2krr3/pppn3p/3b2p1/P1n1P3/b1P5/P3BN2/4BP1P/1R3RK1 b - - 0 21 c0 12144128344513551629; c1 454881177368838468; c2 1477747400540752; c3 0",
			"2krr3/ppp4p/3b2p1/P1n1n3/b1P5/P3BN2/4BP1P/1R3RK1 w - - 0 22 c0 5370688116669259020; c1 28430073586732052; c2 96757259044901; c3 0",
			"2krr3/ppp4p/3b2p1/P1n1n3/b1PN4/P3B3/4BP1P/1R3RK1 b - - 1 22 c0 5370688116669259021; c1 22535694829440020; c2 96825978521637; c3 0",
			"2krr3/1pp4p/p2b2p1/P1n1n3/b1PN4/P3B3/4BP1P/1R3RK1 w - - 0 23 c0 5373225665220549900; c1 22535694829440020; c2 101155305556005; c3 0",
			"2krr3/1pp4p/p2b2p1/P1n1n3/bRPN4/P3B3/4BP1P/5RK1 b - - 1 23 c0 5373225665220549901; c1 360571115725343764; c2 101224024146512; c3 0",
			"2krr3/1pp5/p2b2p1/P1n1n2p/bRPN4/P3B3/4BP1P/5RK1 w - - 0 24 c0 4947503559038704908; c1 360571115733071041; c2 105553351180880; c3 0",
			"2krr3/1pp5/p2b2p1/P1n1n2p/bRP5/P3BN2/4BP1P/5RK1 b - - 1 24 c0 4947503559038704909; c1 454881175849743553; c2 105622070657616; c3 0",
			"2krr3/1pp5/p2b2p1/P1n4p/bRP3n1/P3BN2/4BP1P/5RK1 w - - 2 25 c0 4947503559038704908; c1 454886378561897665; c2 110088836645456; c3 0",
			"2krr3/1pp5/p2b2p1/P1n4p/b1P3n1/P3BN2/1R2BP1P/5RK1 b - - 3 25 c0 4947503559038704909; c1 16169331463162463425; c2 110157556122192; c3 0",
			"1k1rr3/1pp5/p2b2p1/P1n4p/b1P3n1/P3BN2/1R2BP1P/5RK1 w - - 4 26 c0 4947503559038704780; c1 16169331463162463425; c2 114624322110032; c3 0",
			"1k1rr3/1pp5/p2b2p1/P1n3Bp/b1P3n1/P4N2/1R2BP1P/5RK1 b - - 5 26 c0 4947503559038704781; c1 16167945478561416385; c2 114693041586768; c3 0",
			"1k2r3/1ppr4/p2b2p1/P1n3Bp/b1P3n1/P4N2/1R2BP1P/5RK1 w - - 6 27 c0 4947504066928778380; c1 16167945478561416385; c2 119159807574608; c3 0",
			"1k2r3/1ppr4/p2b2p1/P1n3Bp/b1P3n1/P4N2/1R2BPKP/5R2 b - - 7 27 c0 4947504066928778397; c1 16167945478561416385; c2 119228409594448; c3 0",
			"1k2r3/1ppr4/p2b2p1/P5Bp/b1P1n1n1/P4N2/1R2BPKP/5R2 w - - 8 28 c0 4947504066928778396; c1 16167945558820426753; c2 123695175582288; c3 0",
			"1k2r3/1ppr4/p2b2p1/P5Bp/b1P1n1n1/P4N2/1R3PKP/3B1R2 b - - 9 28 c0 4947504066928778397; c1 16167945558820426753; c2 123763896353824; c3 0",
			"1k2r3/1ppr4/p2b2p1/P5Bp/b1P3n1/P1n2N2/1R3PKP/3B1R2 w - - 10 29 c0 4947504066928778396; c1 16169261094418289665; c2 128230662341664; c3 0",
			"1k2r3/1ppr4/p2b2p1/P5Bp/b1P3n1/P1n2N2/5PKP/1R1B1R2 b - - 11 29 c0 4947504066928778397; c1 28360029922432001; c2 128299381846082; c3 0",
			"1k2r3/1ppr4/p1bb2p1/P5Bp/2P3n1/P1n2N2/5PKP/1R1B1R2 w - - 12 30 c0 5392234530131614876; c1 28360029908844564; c2 132766147833922; c3 0",
			"1k2r3/1ppr4/p1bb2p1/P5Bp/2P3n1/PRn2N2/5PKP/3B1R2 b - - 13 30 c0 5392234530131614877; c1 453985379067445268; c2 132834867282976; c3 0",
			"1k2r3/1ppr4/p1bb2p1/P5Bp/2P3n1/PR3N2/5PKP/3n1R2 w - - 0 31 c0 5392234530131614876; c1 27269314374090772; c2 8521222766658; c3 0",
			"1k2r3/1ppr4/p1bb2p1/P5Bp/2P3n1/PR3N1P/5PK1/3n1R2 b - - 0 31 c0 5392234530131614877; c1 603730066677514260; c2 8521222766624; c3 0",
			"1k2r3/1ppr4/p2b2p1/P5Bp/b1P3n1/PR3N1P/5PK1/3n1R2 w - - 1 32 c0 4947504066928778396; c1 603730066691101697; c2 8800395640864; c3 0",
			"1k2r3/1ppr4/p2b2p1/P5Bp/b1P3n1/P4N1P/5PK1/1R1n1R2 b - - 2 32 c0 4947504066928778397; c1 37719072898061313; c2 8804690609922; c3 0",
			"1k2r3/1ppr4/p2b2p1/P3n1Bp/b1P5/P4N1P/5PK1/1R1n1R2 w - - 3 33 c0 4947504066928778396; c1 37718747825918721; c2 9083863484162; c3 0",
			"1k2r3/1ppr4/p2b2p1/P3n1Bp/b1P5/P4N1P/5PK1/1R1R4 b - - 0 33 c0 4947504066928778397; c1 37718747825918721; c2 566935799554; c3 0",
			"1k2r3/1ppr4/p2b2p1/P5Bp/b1P5/P4n1P/5PK1/1R1R4 w - - 0 34 c0 4947504066928778396; c1 2308763380906234881; c2 36507229296; c3 0",
			"1k2r3/1ppr4/p2R2p1/P5Bp/b1P5/P4n1P/5PK1/1R6 b - - 0 34 c0 4915978869537184925; c1 2308763380906234881; c2 2281701488; c3 0",
			"1k2r3/1ppr4/p2R2p1/P5np/b1P5/P6P/5PK1/1R6 w - - 0 35 c0 4915978869537184924; c1 144255994373393409; c2 146800647; c3 0",
			"1k2r3/1ppr4/R5p1/P5np/b1P5/P6P/5PK1/1R6 b - - 0 35 c0 1459290233669885085; c1 8079466531896265920; c2 9175040; c3 0",
			"1k4r1/1ppr4/R5p1/P5np/b1P5/P6P/5PK1/1R6 w - - 1 36 c0 1459290233674407068; c1 8079466531896265920; c2 9441280; c3 0",
			"1k4r1/1ppr4/R5p1/P1P3np/b7/P6P/5PK1/1R6 b - - 0 36 c0 1459290233674407069; c1 8079466531913255940; c2 9437184; c3 0",
			"1k4r1/1p1r4/R1p3p1/P1P3np/b7/P6P/5PK1/1R6 w - - 0 37 c0 1460370294469041308; c1 8079466531913255940; c2 9699328; c3 0",
			"1k4r1/1p1r4/1Rp3p1/P1P3np/b7/P6P/5PK1/1R6 b - - 1 37 c0 1460377991050435741; c1 8079466531913255940; c2 9703424; c3 0",
			"1k1r4/1p1r4/1Rp3p1/P1P3np/b7/P6P/5PK1/1R6 w - - 2 38 c0 1460377991045160092; c1 8079466531913255940; c2 9969664; c3 0",
			"1k1r4/1p1r4/PRp3p1/2P3np/b7/P6P/5PK1/1R6 b - - 0 38 c0 4919304133074984093; c1 8079466531913255940; c2 9961472; c3 0",
			"1k1r4/1p1r4/PRp3p1/1bP3np/8/P6P/5PK1/1R6 w - - 1 39 c0 4919304133074984092; c1 8079466531899883626; c2 10227712; c3 0",
			"1k1r4/1p1r4/PRp3p1/1bP3np/1R6/P6P/5PK1/8 b - - 2 39 c0 4919304133074984093; c1 144255994527793258; c2 10231808; c3 0",
			"1k1r4/1p1r4/PRp1n1p1/1bP4p/1R6/P6P/5PK1/8 w - - 3 40 c0 6000168043643903132; c1 144255994527483556; c2 10498048; c3 0",
			"1k1r4/1p1r4/PRp1n1p1/1RP4p/8/P6P/5PK1/8 b - - 0 40 c0 6000168043643903133; c1 9015999651644644; c2 655360; c3 0",
			"1k1r4/1p1r4/PRp3p1/1RP4p/5n2/P6P/5PK1/8 w - - 1 41 c0 4919304133074984092; c1 9016000280821838; c2 672000; c3 0",
			"1k1r4/1p1r4/PRp3p1/1RP4p/5n2/P4K1P/5P2/8 b - - 2 41 c0 4919304133074984107; c1 4508002606940238; c2 672256; c3 0",
			"1k1r4/1p1r4/PR4p1/1pP4p/5n2/P4K1P/5P2/8 w - - 0 42 c0 2612335223954447530; c1 281750162933766; c2 43008; c3 0",
			"1k1r4/1p1r4/PR4p1/1pP4p/5K2/P6P/5P2/8 b - - 0 42 c0 2612335223954447547; c1 17609374337030; c2 2688; c3 0",
			"1k1r4/1p3r2/PR4p1/1pP4p/5K2/P6P/5P2/8 w - - 1 43 c0 2612335372130819258; c1 17609374337030; c2 2753; c3 0",
			"1k1r4/1p3r2/PR4p1/1pP3Kp/8/P6P/5P2/8 b - - 2 43 c0 2612335372130819277; c1 17609374319622; c2 2754; c3 0",
			"1k1r4/1p6/PR4p1/1pP2rKp/8/P6P/5P2/8 w - - 3 44 c0 7080799976586714316; c1 17609374320352; c2 2819; c3 0",
			"1k1r4/1p6/PR4K1/1pP2r1p/8/P6P/5P2/8 b - - 0 44 c0 220691844194606301; c1 1100585894999; c2 176; c3 0",
			"1k1r4/1p6/PR4K1/1pr4p/8/P6P/5P2/8 w - - 0 45 c0 3391225981863435484; c1 4611686087214006340; c2 11; c3 0",
			"1k1r4/1p6/PR4K1/1pr4p/5P2/P6P/8/8 b - - 0 45 c0 3391225981863435485; c1 4611686019501658180; c2 11; c3 0",
			"3r4/kp6/PR4K1/1pr4p/5P2/P6P/8/8 w - - 1 46 c0 3391225981864187996; c1 9295429631966974020; c2 11; c3 0",
			"3r4/kR6/P5K1/1pr4p/5P2/P6P/8/8 b - - 0 46 c0 4823636744846383197; c1 13258597303045882116; c2 0; c3 0",
			"3r4/1R6/k5K1/1pr4p/5P2/P6P/8/8 w - - 0 47 c0 4913163311141819484; c1 846676729949849616; c2 0; c3 0",
			"3r4/5R2/k5K1/1pr4p/5P2/P6P/8/8 b - - 1 47 c0 4913163318188250205; c1 846958204926560272; c2 0; c3 0",
			"6r1/5R2/k5K1/1pr4p/5P2/P6P/8/8 w - - 2 48 c0 4913163318198801500; c1 865254078412752912; c2 0; c3 0",
			"6r1/6R1/k5K1/1pr4p/5P2/P6P/8/8 b - - 3 48 c0 4913163325714994269; c1 865535553389463568; c2 0; c3 0",
			"5r2/6R1/k5K1/1pr4p/5P2/P6P/8/8 w - - 4 49 c0 4913163325708964956; c1 883831426875656208; c2 0; c3 0",
			"5r2/7R/k5K1/1pr4p/5P2/P6P/8/8 b - - 5 49 c0 4913163340741350493; c1 884112901852366864; c2 0; c3 0",
			"8/7R/k5K1/1pr4p/5r2/P6P/8/8 w - - 0 50 c0 307072708795962460; c1 56294995342393495; c2 0; c3 0",
			"8/8/k5K1/1pr4R/5r2/P6P/8/8 b - - 0 50 c0 8078383577361814621; c1 3518437208899593; c2 0; c3 0",
			"8/8/k4rK1/1pr4R/8/P6P/8/8 w - - 1 51 c0 126928919390590044; c1 3589905464705032; c2 0; c3 0",
			"8/8/k4K2/1pr4R/8/P6P/8/8 b - - 0 51 c0 9231305081968661595; c1 224300372067328; c2 0; c3 0",
			"8/8/k4K2/1p5r/8/P6P/8/8 w - - 0 52 c0 578080401650619482; c1 14293651161152; c2 0; c3 0",
			"8/8/k4K2/1p5r/P7/7P/8/8 b - - 0 52 c0 3871449160881243; c1 14293651161152; c2 0; c3 0",
			"8/8/k4K2/1p6/P7/7r/8/8 w - - 0 53 c0 141905719465050; c1 910533066844; c2 0; c3 0",
			"8/8/k4K2/1P6/8/7r/8/8 b - - 0 53 c0 13835058124001645659; c1 56908316677; c2 0; c3 0",
			"8/8/5K2/1k6/8/7r/8/8 w - - 0 54 c0 6629298651489374426; c1 3623878656; c2 0; c3 0",
			"8/8/8/1k4K1/8/7r/8/8 b - - 1 54 c0 6629298651489374413; c1 3624927232; c2 0; c3 0",
			"8/8/8/1k4K1/8/r7/8/8 w - - 2 55 c0 51791395714765004; c1 3693084672; c2 0; c3 0",
			"8/8/8/1k3K2/8/r7/8/8 b - - 3 55 c0 51791395714765003; c1 3694133248; c2 0; c3 0",
			"8/8/8/5K2/2k5/r7/8/8 w - - 4 56 c0 51791395714764106; c1 3762290688; c2 0; c3 0",
			"8/8/8/8/2k1K3/r7/8/8 b - - 5 56 c0 51791395714764089; c1 3763339264; c2 0; c3 0",
			"r7/8/8/8/2k1K3/8/8/8 w - - 6 57 c0 191800; c1 3831496704; c2 0; c3 0",
			"r7/8/8/5K2/2k5/8/8/8 b - - 7 57 c0 191819; c1 3832545280; c2 0; c3 0",
			"r7/8/8/5K2/3k4/8/8/8 w - - 8 58 c0 191946; c1 3900702720; c2 0; c3 0",
			"r7/8/4K3/8/3k4/8/8/8 b - - 9 58 c0 191961; c1 3901751296; c2 0; c3 0",
			"7r/8/4K3/8/3k4/8/8/8 w - - 10 59 c0 24120792; c1 3969908736; c2 0; c3 0",
			"7r/8/8/5K2/3k4/8/8/8 b - - 11 59 c0 24120779; c1 3970957312; c2 0; c3 0",
			"8/8/7r/5K2/3k4/8/8/8 w - - 12 60 c0 1580547968458; c1 4039114752; c2 0; c3 0",
			"8/8/7r/6K1/3k4/8/8/8 b - - 13 60 c0 1580547968461; c1 4040163328; c2 0; c3 0",
			"8/8/8/6K1/3k4/8/8/7r w - - 14 61 c0 3532; c1 4108344320; c2 0; c3 0",
			"8/8/8/8/3k1K2/8/8/7r b - - 15 61 c0 3515; c1 4109392896; c2 0; c3 0",
			"8/8/8/8/3k1K2/8/8/5r2 w - - 16 62 c0 3514; c1 4177532672; c2 0; c3 0",
			"8/8/8/6K1/3k4/8/8/5r2 b - - 17 62 c0 3533; c1 4178581248; c2 0; c3 0",
			"8/8/8/4k1K1/8/8/8/5r2 w - - 18 63 c0 4684; c1 4246738688; c2 0; c3 0",
			"8/8/8/4k3/6K1/8/8/5r2 b - - 19 63 c0 4669; c1 4247787264; c2 0; c3 0",
			"8/8/8/4k3/5rK1/8/8/8 w - - 20 64 c0 12947848928694844; c1 4315938816; c2 0; c3 0",
			"8/8/8/4k1K1/5r2/8/8/8 b - - 21 64 c0 6473924464349773; c1 4316987392; c2 0; c3 0",
			"r1bqkb1r/pppp1ppn/2n1p2p/3P4/2P1P3/2N2N1P/PP3PP1/R1BQKB1R b KQkq - 0 8 c0 2938090484954496521; c1 13839632060313291206; c2 14215971781816158304; c3 131193",
			"r1bqkb1r/pppp1ppn/4p2p/n2P4/2P1P3/2N2N1P/PP3PP1/R1BQKB1R w KQkq - 1 9 c0 2938090484954496520; c1 13839632063257446854; c2 14215971781816158304; c3 147833",
			"r1bqkb1r/pppp1ppn/4p2p/n2P4/2P1P3/2N2N1P/PP3PPR/R1BQKB2 b Qkq - 2 9 c0 2938090484954496521; c1 13839632063257446854; c2 6094623207051036768; c3 148080",
			"r1bqk2r/pppp1ppn/4p2p/n2P4/1bP1P3/2N2N1P/PP3PPR/R1BQKB2 w Qkq - 3 10 c0 7101159682649816584; c1 13839678205617311900; c2 6094623207051036768; c3 164720",
			"r1bqk2r/pppp1ppn/4p2B/n2P4/1bP1P3/2N2N1P/PP3PPR/R2QKB2 b Qkq - 0 10 c0 7101159682649816585; c1 13839678205611020444; c2 380681179824719968; c3 10247",
			"r1bqk2r/pppp1p1n/4p2p/n2P4/1bP1P3/2N2N1P/PP3PPR/R2QKB2 w Qkq - 0 11 c0 11712845701077204488; c1 864979887851081993; c2 8094243105986973830; c3 704",
			"r1bqk2r/pppp1p1n/4p2p/n2P4/1bP1P3/2N2NPP/PP3P1R/R2QKB2 b Qkq - 0 11 c0 11712845701077204489; c1 864979887851081993; c2 8094243105418577990; c3 704",
			"r1bqk2r/pppp1p2/4p2p/n2P2n1/1bP1P3/2N2NPP/PP3P1R/R2QKB2 w Qkq - 1 12 c0 2489473664222428680; c1 864979908107143440; c2 8094243105418577990; c3 769",
			"r1bqk2r/pppp1p2/4p2p/n2PP1n1/1bP5/2N2NPP/PP3P1R/R2QKB2 b Qkq - 0 12 c0 2489473664222428681; c1 864808003383298320; c2 8094243105418577990; c3 768",
			"r1bqk2r/p1pp1p2/1p2p2p/n2PP1n1/1bP5/2N2NPP/PP3P1R/R2QKB2 w Qkq - 0 13 c0 2461433918690885128; c1 864808003383298322; c2 8094243105418577990; c3 832",
			"r1bqk2r/p1pp1p2/1p2p2p/n2PP1n1/1bP4P/2N2NP1/PP3P1R/R2QKB2 b Qkq - 0 13 c0 2461433918690885129; c1 13871203727229290770; c2 8094243105418576992; c3 832",
			"r1bqk2r/p1pp1p2/1p2p2p/n2PP1n1/2P4P/2b2NP1/PP3P1R/R2QKB2 w Qkq - 0 14 c0 2461433918690885128; c1 6055094423741761810; c2 505890194088661062; c3 56",
			"r1bqk2r/p1pp1p2/1p2p2p/n2PP1n1/2P4P/2P2NP1/P4P1R/R2QKB2 b Qkq - 0 14 c0 2461433918690885129; c1 290486900707526930; c2 9254990173985312838; c3 3",
			"r1bqk2r/p1pp1p2/1p2p2p/n2PP3/2P4P/2P2nP1/P4P1R/R2QKB2 w Qkq - 0 15 c0 2461433918690885128; c1 6935684438794930450; c2 4325431775846334726; c3 0",
			"r1bqk2r/p1pp1p2/1p2p2p/n2PP3/2P4P/2P2QP1/P4P1R/R3KB2 b Qkq - 0 15 c0 2461433918690885129; c1 2323998420367542546; c2 270339468479824133; c3 0",
			"r2qk2r/pbpp1p2/1p2p2p/n2PP3/2P4P/2P2QP1/P4P1R/R3KB2 w Qkq - 1 16 c0 2461435662982905352; c1 2323998420367542546; c2 288635341966016773; c3 0",
			"r2qk2r/pbpp1p2/1p2p2p/n2PP3/2P4P/2P1Q1P1/P4P1R/R3KB2 b Qkq - 2 16 c0 2461435662982905353; c1 10394448952615471378; c2 288916816942727428; c3 0",
			"r3k2r/pbppqp2/1p2p2p/n2PP3/2P4P/2P1Q1P1/P4P1R/R3KB2 w Qkq - 3 17 c0 2466156662613016072; c1 10394448952615471378; c2 307212690428920068; c3 0",
			"r3k2r/pbppqp2/1p2p2p/n2PP3/2PQ3P/2P3P1/P4P1R/R3KB2 b Qkq - 4 17 c0 2466156662613016073; c1 290561615958608146; c2 307494165405630724; c3 0",
			"r3k2r/pbpp1p2/1p2p2p/n2PP3/2PQ3P/q1P3P1/P4P1R/R3KB2 w Qkq - 5 18 c0 2459682738148670984; c1 400966069323905169; c2 325790038891823364; c3 0",
			"r3k2r/pbpp1p2/1p2p2p/n2PP3/2PQ3P/q1P3P1/P4P1R/3RKB2 b kq - 6 18 c0 2459682738148670985; c1 400966069323905169; c2 326053934835826948; c3 0",
			"2kr3r/pbpp1p2/1p2p2p/n2PP3/2PQ3P/q1P3P1/P4P1R/3RKB2 w - - 7 19 c0 2459682738149235976; c1 400966069323905169; c2 344244255205753092; c3 0",
			"2kr3r/pbpp1p2/1p2p2p/n2PP3/2P4P/q1P3P1/P2Q1P1R/3RKB2 b - - 8 19 c0 2459682738149235977; c1 4636746104644909201; c2 344525730182481936; c3 0",
			"1k1r3r/pbpp1p2/1p2p2p/n2PP3/2P4P/q1P3P1/P2Q1P1R/3RKB2 w - - 9 20 c0 2459682738149235848; c1 4636746104644909201; c2 362821603668674576; c3 0",
			"1k1r3r/pbpp1p2/1p1Pp2p/n3P3/2P4P/q1P3P1/P2Q1P1R/3RKB2 b - - 0 20 c0 11683054775004011657; c1 4636746104645585168; c2 360288328878278672; c3 0",
			"1kr4r/pbpp1p2/1p1Pp2p/n3P3/2P4P/q1P3P1/P2Q1P1R/3RKB2 w - - 1 21 c0 11683054775003634824; c1 4636746104645585168; c2 378584202364471312; c3 0",
			"1kr4r/pbpp1p2/1p1Pp2p/n3P3/2P2P1P/q1P3P1/P2Q3R/3RKB2 b - - 0 21 c0 11683054775003634825; c1 400963320545511696; c2 378302727387906308; c3 0",
			"1kr3r1/pbpp1p2/1p1Pp2p/n3P3/2P2P1P/q1P3P1/P2Q3R/3RKB2 w - - 1 22 c0 11683054774907165832; c1 400963320545511696; c2 396598600874098948; c3 0",
			"1kr3r1/pbpp1p2/1p1Pp2p/n3P3/2P2P1P/q1P3PR/P2Q4/3RKB2 b - - 2 22 c0 11683054774907165833; c1 400963320545511696; c2 396880075796517764; c3 0",
			"1kr3r1/p1pp1p2/1p1Pp2p/n3P3/2P2P1P/q1P2bPR/P2Q4/3RKB2 w - - 3 23 c0 730190916526398600; c1 11554275253602564241; c2 415175949282710406; c3 0",
			"1kr3r1/p1pp1p2/1p1Pp2p/n3P3/2P2P1P/q1P2bPR/P2Q4/1R2KB2 b - - 4 23 c0 730190916526398601; c1 11554275253602564241; c2 415457412985131910; c3 0",
			"1kr3r1/p2p1p2/1p1pp2p/n3P3/2P2P1P/q1P2bPR/P2Q4/1R2KB2 w - - 0 24 c0 1775019175308549256; c1 7639671230991242121; c2 27021619474235704; c3 0",
			"1kr3r1/p2p1p2/1p1pp2p/n3P3/2P2P1P/q1P2bPR/P2Q1K2/1R3B2 b - - 1 24 c0 1775019175308549275; c1 7639671230991242121; c2 27039211542839608; c3 0",
			"1kr3r1/p2p1p2/1p1pp2p/n3P3/2P1bP1P/q1P3PR/P2Q1K2/1R3B2 w - - 2 25 c0 1775019175308549274; c1 4636746315148104585; c2 28182703635726648; c3 0",
			"1kr3r1/p2p1p2/1p1pp2p/n3P3/1RP1bP1P/q1P3PR/P2Q1K2/5B2 b - - 3 25 c0 1775019175308549275; c1 400964747765289865; c2 28200295708758916; c3 0",
			"1kr3r1/p2p1p2/1p2p2p/n2pP3/1RP1bP1P/q1P3PR/P2Q1K2/5B2 w - - 0 26 c0 10457959256878865562; c1 400964747765319992; c2 29273419057468292; c3 0",
			"1kr3r1/p2p1p2/1p2p2p/n2pP3/1RP1bP1P/q1P3PR/P2Q4/4KB2 b - - 1 26 c0 10457959256878865545; c1 400964747765319992; c2 29291011243512708; c3 0",
			"1kr3r1/p2p1p2/1p2p2p/3pP3/1Rn1bP1P/q1P3PR/P2Q4/4KB2 w - - 0 27 c0 10457959256878865544; c1 4636746316370679944; c2 1899957435269432; c3 0",
			"1kr3r1/p2p1p2/1p2p2p/3pP3/1RB1bP1P/q1P3PR/P2Q4/4K3 b - - 0 27 c0 10457959256878865545; c1 4636746315431155848; c2 118747256095032; c3 0",
			"1kr3r1/p2p1p2/1p2p2p/4P3/1Rp1bP1P/q1P3PR/P2Q4/4K3 w - - 0 28 c0 10457959256878865544; c1 9513168681619554568; c2 7696581412883; c3 0",
			"1kr3r1/p2p1p2/1p2p2p/4P3/1Rp1bP1P/q1P3PR/PQ6/4K3 b - - 1 28 c0 10457959256878865545; c1 9513168681619554568; c2 7700876366355; c3 0",
			"1kr3r1/p2p1p2/1p2p2p/4P3/1Rp1bP1P/2P3PR/Pq6/4K3 w - - 0 29 c0 10457959256878865544; c1 4053310597432738056; c2 498216207137; c3 0",
			"1kr3r1/p2p1p2/1p2p2p/4P3/2p1bP1P/2P3PR/PR6/4K3 b - - 0 29 c0 10457959256878865545; c1 1406253416946336008; c2 31138512910; c3 0",
			"1kr3r1/p2p1p2/1p2p2p/4Pb2/2p2P1P/2P3PR/PR6/4K3 w - - 1 30 c0 10457959256878865544; c1 1406253416661033224; c2 32229031950; c3 0",
			"1kr3r1/p2p1p2/1p2p2p/4Pb2/2p2P1P/2P3P1/PR5R/4K3 b - - 2 30 c0 10457959256878865545; c1 16214088991455224072; c2 32245810048; c3 0",
			"1kr5/p2p1p2/1p2p2p/4Pb2/2p2P1P/2P3r1/PR5R/4K3 w - - 0 31 c0 9876994490404035720; c1 1014928674337860112; c2 2080374840; c3 0",
			"1kr5/p2p1p2/1p2p2p/4Pb2/2p2P1P/2P3r1/P2R3R/4K3 b - - 1 31 c0 9876994490404035721; c1 4041347623930833424; c2 2081423416; c3 0",
			"1kr5/p2p1p2/1p2p2p/4Pb2/2p2P1P/2r5/P2R3R/4K3 w - - 0 32 c0 9876994490404035720; c1 9475861415341599248; c2 134217731; c3 0",
			"1kr5/p2p1p2/1p2p2p/4Pb2/2p2P1P/2r5/P2R1K1R/8 b - - 1 32 c0 9876994490404035739; c1 14087547433768987152; c2 134283265; c3 0",
			"1kr5/p2p1p2/1p2p2p/4P3/2p1bP1P/2r5/P2R1K1R/8 w - - 2 33 c0 9876994490404035738; c1 14087547433786818576; c2 138543105; c3 0",
			"1kr5/p2p1p2/1p2p2p/4P3/2p1bP1P/2r5/P2R3R/4K3 b - - 3 33 c0 9876994490404035721; c1 9475861415359430672; c2 138608643; c3 0",
			"1kr5/p2p1p2/1p2p2p/4P3/2p1bP1P/5r2/P2R3R/4K3 w - - 4 34 c0 9876994490404035720; c1 9475905670702448656; c2 142868483; c3 0",
			"1kr5/p2p1p2/1p2p2p/4P3/2p1bP1P/5r2/P4R1R/4K3 b - - 5 34 c0 9876994490404035721; c1 10232510408100691984; c2 142934019; c3 0",
			"2r5/p1kp1p2/1p2p2p/4P3/2p1bP1P/5r2/P4R1R/4K3 w - - 6 35 c0 9876994490689624328; c1 10232510408100691984; c2 147193859; c3 0",
			"2r5/p1kp1p2/1p2p2p/4P3/2p1bP1P/5r2/P3KR1R/8 b - - 7 35 c0 9876994490689624345; c1 14339793268262584336; c2 147259393; c3 0",
			"2r5/p1kp1p2/1p2p2p/4P3/2p1bP1P/r7/P3KR1R/8 w - - 8 36 c0 9876994490689624344; c1 14339744271275671568; c2 151519233; c3 0",
			"2r5/p1kp1p2/1p2p2p/4P3/2p1bP1P/r7/P4R1R/4K3 b - - 9 36 c0 9876994490689624329; c1 10232461411113779216; c2 151584771; c3 0",
			"2r5/p1k2p2/1p1pp2p/4P3/2p1bP1P/r7/P4R1R/4K3 w - - 0 37 c0 9878824050120956168; c1 10232461411113779216; c2 155189251; c3 0",
			"2r5/p1k2p2/1p1pp2p/4P3/2p1bP1P/r7/P6R/4KR2 b - - 1 37 c0 9878824050120956169; c1 4035508323851976720; c2 155254896; c3 0",
			"2r5/p1k2p2/1p1pp2p/4P3/4bP1P/r1p5/P6R/4KR2 w - - 0 38 c0 9878824050120956168; c1 4035511512947032080; c2 159383664; c3 0",
			"2r5/p1k2p2/1p1pp2p/4P3/4bP1P/r1p5/P4R2/4KR2 b - - 1 38 c0 9878824050120956169; c1 1009092563354058768; c2 159449200; c3 0",
			"2r5/2k2p2/1p1pp2p/p3P3/4bP1P/r1p5/P4R2/4KR2 w - - 0 39 c0 1770348007704467720; c1 1009092563354058769; c2 163577968; c3 0",
			"2r5/2k2p2/1p1pp2p/p3P3/4bP1P/r1p5/P4R2/3K1R2 b - - 1 39 c0 1770348007704467719; c1 1009092563354058769; c2 163643504; c3 0",
			"2r5/2k2p2/1p1pp2p/p3P3/4bP1P/r7/P1p2R2/3K1R2 w - - 0 40 c0 1770348007704467718; c1 1027964306055299089; c2 167772272; c3 0",
			"2r5/2k2p2/1p1pp2p/p3P3/4bP1P/r7/P1p2R2/2K2R2 b - - 1 40 c0 1770348007704467717; c1 1027964306055299089; c2 167837808; c3 0",
			"2r5/2k2p2/1p1pp2p/p3P3/4bP1P/8/r1p2R2/2K2R2 w - - 0 41 c0 1770348007704467716; c1 64271952339337233; c2 10747911; c3 0",
			"2r5/2k2p2/1p1pp2p/p3P3/4bP1P/8/r1pK1R2/5R2 b - - 1 41 c0 1770348007704467735; c1 32746754947743761; c2 10752007; c3 0",
			"2r5/3k1p2/1p1pp2p/p3P3/4bP1P/8/r1pK1R2/5R2 w - - 2 42 c0 1770348007704467862; c1 32746754947743761; c2 11018247; c3 0",
			"2r5/3k1p2/1p1pp2p/p3P3/4bP1P/8/r1pK1R2/2R5 b - - 3 42 c0 1770348007704467863; c1 16173647819443601425; c2 11022336; c3 0",
			"8/3k1p2/1prpp2p/p3P3/4bP1P/8/r1pK1R2/2R5 w - - 4 43 c0 1770352130905872790; c1 16173647819443601425; c2 11288576; c3 0",
			"8/3k1p2/1prpp2p/p3P3/4bP1P/8/r1p2R2/2R1K3 b - - 5 43 c0 1770352130905872777; c1 13899330007621500945; c2 11292673; c3 0",
			"8/3k1p2/1pr1p2p/p2pP3/4bP1P/8/r1p2R2/2R1K3 w - - 0 44 c0 10486937268286134664; c1 13899330007621500952; c2 11534337; c3 0",
			"8/3k1p2/1pr1p2p/p2pP3/4bP1P/8/r1p1KR2/2R5 b - - 1 44 c0 10486937268286134681; c1 16173647819443601432; c2 11538432; c3 0",
			"8/3k1p2/1pr1p2p/p2pP3/4bP1P/8/1rp1KR2/2R5 w - - 2 45 c0 10486937268286134680; c1 16173673108211040280; c2 11804672; c3 0",
			"8/3k1p2/1pr1p2p/p2pP2P/4bP2/8/1rp1KR2/2R5 b - - 0 45 c0 10486937268286134681; c1 16173673108128925720; c2 11796480; c3 0",
			"8/3k1p2/1p2p2p/p2pP2P/4bP2/2r5/1rp1KR2/2R5 w - - 1 46 c0 9878805358422530456; c1 16173673132772884609; c2 12062720; c3 0",
			"8/3k1p2/1p2p2p/p2pP2P/4bP2/2r5/1rpK1R2/2R5 b - - 2 46 c0 9878805358422530455; c1 16173673132772884609; c2 12066816; c3 0",
			"8/3k1p2/1p2p2p/p2pP2P/4bP2/7r/1rpK1R2/2R5 w - - 3 47 c0 9878805358422530454; c1 16173673898350805121; c2 12333056; c3 0",
			"8/3k1p2/1p2p2p/p2pP2P/4bP2/7r/1rpK4/2R2R2 b - - 4 47 c0 9878805358422530455; c1 1010053952994345089; c2 12337159; c3 0",
			"8/3k1p2/1p2p2p/p2pP2r/4bP2/8/1rpK4/2R2R2 w - - 0 48 c0 9878805358422530454; c1 8133578854921210753; c2 786432; c3 0",
			"8/3k1p2/1p2p2p/p2pP2r/4bP2/4K3/1rp5/2R2R2 b - - 1 48 c0 9878805358422530473; c1 8133539890977901441; c2 786688; c3 0",
			"8/3k1p2/1p2p2p/p2pP3/4bP2/4K2r/1rp5/2R2R2 w - - 2 49 c0 9878805358422530472; c1 8133539915670704129; c2 803328; c3 0",
			"8/3k1p2/1p2p2p/p2pP3/4bP2/7r/1rp1K3/2R2R2 b - - 3 49 c0 9878805358422530457; c1 8133578904310075393; c2 803584; c3 0",
			"8/3k1p2/1p2p2p/p2pP3/5P2/3b3r/1rp1K3/2R2R2 w - - 4 50 c0 9878805358422530456; c1 8133578904486027265; c2 820224; c3 0",
			"8/3k1p2/1p2p2p/p2pP3/5P2/3b3r/1rpK4/2R2R2 b - - 5 50 c0 9878805358422530455; c1 8133578904486027265; c2 820480; c3 0",
			"8/3k1p2/1p2p2p/p2pP3/5P2/7r/1rpK4/2R2b2 w - - 0 51 c0 9878805358422530454; c1 1517154998050365441; c2 52224; c3 0",
			"8/3k1p2/1p2p2p/p2pP3/5P2/7r/1rpK4/5R2 b - - 0 51 c0 9878805358422530455; c1 31530070971523073; c2 3264; c3 0",
			"8/3k1p2/1p2p2p/p2pP3/5P2/8/1rpK3r/5R2 w - - 1 52 c0 9878805358422530454; c1 31626656867164161; c2 3329; c3 0",
			"8/3k1p2/1p2p2p/p2pP3/5P2/2K5/1rp4r/5R2 b - - 2 52 c0 9878805358422530469; c1 31626504664260609; c2 3330; c3 0",
			"8/3k1p2/1p2p2p/p2pP3/5P2/2K5/2p4r/1r3R2 w - - 3 53 c0 9878805358422530468; c1 31936148989288449; c2 3395; c3 0",
			"8/3k1p2/1p2p2p/p2pP3/5P2/2K5/2p4r/1rR5 b - - 4 53 c0 9878805358422530469; c1 4351601271644161; c2 3396; c3 0",
			"8/3k1p2/1p2p2p/p2pP3/5P2/2K5/2p4r/2r5 w - - 0 54 c0 9878805358422530468; c1 815571876716545; c2 216; c3 0",
			"8/3k1p2/1p2p2p/p2pPP2/8/2K5/2p4r/2r5 b - - 0 54 c0 9878805358422530469; c1 815571876708385; c2 216; c3 0",
			"8/3k1p2/1p2p2p/p2pPP2/8/2K5/2p4r/5r2 w - - 1 55 c0 9878805358422530468; c1 1159401760389857313; c2 220; c3 0",
			"8/3k1p2/1p2P2p/p2pP3/8/2K5/2p4r/5r2 b - - 0 55 c0 9878787766236486053; c1 13835463071268601857; c2 13; c3 0",
			"8/5p2/1p2k2p/p2pP3/8/2K5/2p4r/5r2 w - - 0 56 c0 1770345955313325604; c1 16140926377995010048; c2 0; c3 0",
			"8/5p2/1p2k2p/p2pP3/3K4/8/2p4r/5r2 b - - 1 56 c0 1770345955313325623; c1 16145429977622380544; c2 0; c3 0",
			"8/5p2/1p2k2p/p2pP3/3K1r2/8/2p4r/8 w - - 2 57 c0 1770345955313325622; c1 16438139035609732864; c2 0; c3 0",
			"8/5p2/1p2k2p/p2pP3/5r2/2K5/2p4r/8 b - - 3 57 c0 1770345955313325605; c1 16442642635237109248; c2 0; c3 0",
			"r1b2knr/pppp1ppp/2n5/2b1p3/4P3/2NP2R1/PPP2PP1/R1BQKBN1 b Q - 2 8 c0 7101159683505454729; c1 13874465656184786060; c2 1181431046615351825; c3 514",
			"r1b2knr/pppp1ppp/8/2b1p3/3nP3/2NP2R1/PPP2PP1/R1BQKBN1 w Q - 3 9 c0 7101159683505454728; c1 13874466304659030156; c2 1181431046615351825; c3 579",
			"r1b2knr/pppp1ppp/8/2b1p3/N2nP3/3P2R1/PPP2PP1/R1BQKBN1 b Q - 4 9 c0 7101159683505454729; c1 13871114903023255692; c2 1181431046615351825; c3 580",
			"r1b2knr/ppppbppp/8/4p3/N2nP3/3P2R1/PPP2PP1/R1BQKBN1 w Q - 5 10 c0 3858567951798697608; c1 13871114903017752774; c2 1181431046615351825; c3 645",
			"r1b2knr/ppppbppp/8/4p3/N2nP3/2PP2R1/PP3PP1/R1BQKBN1 b Q - 0 10 c0 3858567951798697609; c1 594503201529530566; c2 1181431046615343388; c3 640",
			"r1b2knr/ppp1bppp/8/3pp3/N2nP3/2PP2R1/PP3PP1/R1BQKBN1 w Q - 0 11 c0 7157454678847585928; c1 594503201538441356; c2 1181431046615343388; c3 704",
			"r1b2knr/ppp1bppp/8/3Pp3/N2n4/2PP2R1/PP3PP1/R1BQKBN1 b Q - 0 11 c0 7157454678847585929; c1 13872223210739073164; c2 73839440413458961; c3 44",
			"r1b2knr/ppp1b1pp/8/3Ppp2/N2n4/2PP2R1/PP3PP1/R1BQKBN1 w Q - 0 12 c0 14219098894564523656; c1 13872223211041619976; c2 73839440413458961; c3 48",
			"r1b2knr/ppp1b1pp/8/3Ppp2/N2P4/3P2R1/PP3PP1/R1BQKBN1 b Q - 0 12 c0 14219098894564523657; c1 2019864996104667144; c2 4614965025841185; c3 3",
			"r1b2knr/ppp3pp/8/3Ppp2/Nb1P4/3P2R1/PP3PP1/R1BQKBN1 w Q - 1 13 c0 10107312434775260808; c1 2019865163862181888; c2 4688358577491157025; c3 3",
			"r1b2knr/ppp3pp/8/3Ppp2/Nb1P4/3P2R1/PP1B1PP1/R2QKBN1 b Q - 2 13 c0 10107312434775260809; c1 2019865163862181888; c2 4760416167926048801; c3 3",
			"r1b2knr/ppp3pp/8/3Ppp2/N2P4/3P2R1/PP1b1PP1/R2QKBN1 w Q - 0 14 c0 10107312434775260808; c1 1279163066863388672; c2 4035513701212891778; c3 0",
			"r1b2knr/ppp3pp/8/3Ppp2/N2P4/3P2R1/PP1Q1PP1/R3KBN1 b Q - 0 14 c0 10107312434775260809; c1 1279163066863388672; c2 252219605228201090; c3 0",
			"r1b2k1r/ppp1n1pp/8/3Ppp2/N2P4/3P2R1/PP1Q1PP1/R3KBN1 w Q - 1 15 c0 10111502767742451336; c1 1279163066863388672; c2 270515478714393730; c3 0",
			"r1b2k1r/ppp1n1pp/8/3PppQ1/N2P4/3P2R1/PP3PP1/R3KBN1 b Q - 2 15 c0 10111502767742451337; c1 2019864996157130752; c2 270796953691103265; c3 0",
			"r1b2kr1/ppp1n1pp/8/3PppQ1/N2P4/3P2R1/PP3PP1/R3KBN1 w Q - 3 16 c0 10111502766198947464; c1 2019864996157130752; c2 289092827177295905; c3 0",
			"r1b2kr1/ppp1n1pp/8/3PPpQ1/N7/3P2R1/PP3PP1/R3KBN1 b Q - 0 16 c0 10111502766198947465; c1 1279163045186177024; c2 18015525140447746; c3 0",
			"r1b3r1/ppp1nkpp/8/3PPpQ1/N7/3P2R1/PP3PP1/R3KBN1 w Q - 1 17 c0 10116927968567294600; c1 1279163045186177024; c2 19159017233334786; c3 0",
			"r1b3r1/ppp1nkpp/4P3/3P1pQ1/N7/3P2R1/PP3PP1/R3KBN1 b Q - 0 17 c0 10116927968567294601; c1 1279163045186142224; c2 19141425047290370; c3 0",
			"r1b2kr1/ppp1n1pp/4P3/3P1pQ1/N7/3P2R1/PP3PP1/R3KBN1 w Q - 1 18 c0 10111502766198947464; c1 1279163045186142224; c2 20284917140177410; c3 0",
			"r1b2kr1/ppp1n1pp/4P3/3P1pQ1/N7/3P1NR1/PP3PP1/R3KB2 b Q - 2 18 c0 10111502766198947465; c1 2044774981839126544; c2 20302503799431201; c3 0",
			"r1b2kr1/p1p1n1pp/4P3/1p1P1pQ1/N7/3P1NR1/PP3PP1/R3KB2 w Q - 0 19 c0 631968846767128200; c1 2044774981839135233; c2 21393219334184993; c3 0",
			"r1b2kr1/p1p1n1pp/4P3/1p1P1pQ1/8/2NP1NR1/PP3PP1/R3KB2 b Q - 1 19 c0 631968846767128201; c1 2044788163093766657; c2 21410811520229409; c3 0",
			"r4kr1/p1p1n1pp/b3P3/1p1P1pQ1/8/2NP1NR1/PP3PP1/R3KB2 w Q - 2 20 c0 1552707527718928008; c1 2044788163093766657; c2 22554303613116449; c3 0",
			"r4kr1/p1p1n1pp/b3P3/1p1P1pQ1/8/2NP1NR1/PP3PP1/2KR1B2 b - - 3 20 c0 1552707527718928005; c1 2044788163093766657; c2 22570796639854625; c3 0",
			"1r3kr1/p1p1n1pp/b3P3/1p1P1pQ1/8/2NP1NR1/PP3PP1/2KR1B2 w - - 4 21 c0 1552707527719116420; c1 2044788163093766657; c2 23714288732741665; c3 0",
			"1r3kr1/p1p1n1pp/b3P3/1p1P1p1Q/8/2NP1NR1/PP3PP1/2KR1B2 b - - 5 21 c0 1552707527719116421; c1 2044788163697746433; c2 23731880918786081; c3 0",
			"1r3kr1/p1p1n2p/b3P1p1/1p1P1p1Q/8/2NP1NR1/PP3PP1/2KR1B2 w - - 0 22 c0 1249910821522628228; c1 2044788163697746500; c2 24769819895406625; c3 0",
			"1r3kr1/p1p1n2Q/b3P1p1/1p1P1p2/8/2NP1NR1/PP3PP1/2KR1B2 b - - 0 22 c0 1248784921615785605; c1 1280720764795920964; c2 1548113743462914; c3 0",
			"1r3k2/p1p1n1rQ/b3P1p1/1p1P1p2/8/2NP1NR1/PP3PP1/2KR1B2 w - - 1 23 c0 1248866213455847044; c1 1280720764795920964; c2 1619581999268354; c3 0",
			"1r3k2/p1p1n1r1/b3P1p1/1p1P1p2/7Q/2NP1NR1/PP3PP1/2KR1B2 b - - 2 23 c0 4689757066255261317; c1 1280720784089811492; c2 1620681510896130; c3 0",
			"1r3k2/p1p1n1r1/b3P1p1/3P1p2/1p5Q/2NP1NR1/PP3PP1/2KR1B2 w - - 0 24 c0 4689757066255261316; c1 1280720784123371652; c2 1688851231818242; c3 0",
			"1r3k2/p1p1n1r1/b3P1p1/3P1p2/Np5Q/3P1NR1/PP3PP1/2KR1B2 b - - 1 24 c0 4689757066255261317; c1 1280720249937862788; c2 1689950743446018; c3 0",
			"1r4k1/p1p1n1r1/b3P1p1/3P1p2/Np5Q/3P1NR1/PP3PP1/2KR1B2 w - - 2 25 c0 4689757066255261444; c1 1280720249937862788; c2 1761418999251458; c3 0",
			"1r4k1/p1p1n1r1/b3P1p1/2NP1p2/1p5Q/3P1NR1/PP3PP1/2KR1B2 b - - 3 25 c0 4689757066255261445; c1 1280720249936808132; c2 1762518510879234; c3 0",
			"1r4k1/p1p1n1r1/4P1p1/1bNP1p2/1p5Q/3P1NR1/PP3PP1/2KR1B2 w - - 4 26 c0 4904522473485491972; c1 1280720249936808170; c2 1833986766684674; c3 0",
			"1r4k1/p1p1n1r1/4P1p1/1bNP1p2/1Q6/3P1NR1/PP3PP1/2KR1B2 b - - 0 26 c0 4904522473485491973; c1 2385888005775821034; c2 114349295010848; c3 0",
			"6k1/p1p1n1r1/1r2P1p1/1bNP1p2/1Q6/3P1NR1/PP3PP1/2KR1B2 w - - 1 27 c0 4905235639719436036; c1 2385888005775821034; c2 118816060998688; c3 0",
			"6k1/p1p1n1r1/1r2P1p1/QbNP1p2/8/3P1NR1/PP3PP1/2KR1B2 b - - 2 27 c0 4905235639719436037; c1 2385888005507288745; c2 118884780475424; c3 0",
			"6k1/p1p3r1/1r2P1p1/QbNn1p2/8/3P1NR1/PP3PP1/2KR1B2 w - - 0 28 c0 10682870764930080516; c1 149118000344242410; c2 7696586752066; c3 0",
			"6k1/p1p3r1/1r2P1p1/QbNn1p2/3N4/3P2R1/PP3PP1/2KR1B2 b - - 1 28 c0 10682870764930080517; c1 149111919073204458; c2 7700881719362; c3 0",
			"6k1/2p3r1/pr2P1p1/QbNn1p2/3N4/3P2R1/PP3PP1/2KR1B2 w - - 0 29 c0 10682871561632882436; c1 149111919073204458; c2 7971464659010; c3 0",
			"6k1/2p3r1/pr2P1p1/QbNn1p2/P2N4/3P2R1/1P3PP1/2KR1B2 b - - 0 29 c0 10682871561632882437; c1 152005289978271978; c2 7971464659010; c3 0",
			"6k1/2p3r1/prb1P1p1/Q1Nn1p2/P2N4/3P2R1/1P3PP1/2KR1B2 w - - 1 30 c0 4905949508367556356; c1 152005289978271945; c2 8250637533250; c3 0",
			"6k1/2p3r1/Nrb1P1p1/Q2n1p2/P2N4/3P2R1/1P3PP1/2KR1B2 b - - 0 30 c0 4905948546294882053; c1 2315343339837335945; c2 515396410372; c3 0",
			"6k1/1bp3r1/Nr2P1p1/Q2n1p2/P2N4/3P2R1/1P3PP1/2KR1B2 w - - 1 31 c0 4905232929621286660; c1 2315343339837335945; c2 532844715012; c3 0",
			"1N4k1/1bp3r1/1r2P1p1/Q2n1p2/P2N4/3P2R1/1P3PP1/2KR1B2 b - - 2 31 c0 4905235575914684165; c1 2315343339837335945; c2 533113150468; c3 0",
			"1N4k1/1bp3r1/1r2P1p1/Q4p2/P2N1n2/3P2R1/1P3PP1/2KR1B2 w - - 3 32 c0 4905235575914684164; c1 2315343349659279881; c2 550561455108; c3 0",
			"1N4k1/1bp3r1/1r2P1p1/4Qp2/P2N1n2/3P2R1/1P3PP1/2KR1B2 b - - 4 32 c0 4905235575914684165; c1 2315343349659280016; c2 550829890564; c3 0",
			"1N4k1/2p3r1/1r2P1p1/4Qp2/P2N1n2/3P2R1/1P3Pb1/2KR1B2 w - - 0 33 c0 306577223450681092; c1 4756394977781092905; c2 35433501125; c3 0",
			"1N4k1/2p3r1/1r2P1p1/5p2/P2N1Q2/3P2R1/1P3Pb1/2KR1B2 b - - 0 33 c0 306577223450681093; c1 6061882209124581666; c2 2214593820; c3 0",
			"1N4k1/2p3r1/1r2P1p1/5p2/P2N1Q2/3P2R1/1P3P2/2KR1b2 w - - 0 34 c0 306577223450681092; c1 13844102365220798754; c2 142606673; c3 0",
			"6k1/2pN2r1/1r2P1p1/5p2/P2N1Q2/3P2R1/1P3P2/2KR1b2 b - - 1 34 c0 306577222783737605; c1 13844102365220798754; c2 142672209; c3 0",
			"6k1/2pN2r1/4P1p1/5p2/Pr1N1Q2/3P2R1/1P3P2/2KR1b2 w - - 2 35 c0 2325001295427018500; c1 13844102365220810258; c2 146932049; c3 0",
			"6k1/2pN2r1/4P1p1/5p2/Pr1N1Q2/1P1P2R1/5P2/2KR1b2 b - - 0 35 c0 2325001295427018501; c1 13844096075778076178; c2 146800977; c3 0",
			"6k1/2pN2r1/4P1p1/5p2/Pr1N1Q2/1P1b2R1/5P2/2KR4 w - - 0 36 c0 2325001295427018500; c1 13844096762972843538; c2 9437185; c3 0",
			"6k1/2pN2r1/4P1p1/5p2/Pr1N1Q2/1P1R2R1/5P2/2K5 b - - 0 36 c0 2325001295427018501; c1 9038226654342674; c2 589824; c3 0",
			"6k1/2pN2r1/4P1p1/5p2/P2r1Q2/1P1R2R1/5P2/2K5 w - - 0 37 c0 2325001295427018500; c1 564889165936658; c2 37888; c3 0",
			"6k1/2p3r1/4PNp1/5p2/P2r1Q2/1P1R2R1/5P2/2K5 b - - 1 37 c0 2325089985025285893; c1 564889165936658; c2 37904; c3 0",
			"5k2/2p3r1/4PNp1/5p2/P2r1Q2/1P1R2R1/5P2/2K5 w - - 2 38 c0 2325089985025285764; c1 564889165936658; c2 38944; c3 0",
			"5k2/2p3r1/4PNp1/5p2/P2Q4/1P1R2R1/5P2/2K5 b - - 0 38 c0 2325089985025285765; c1 35305572812818; c2 2432; c3 0",
			"5k2/2pr4/4PNp1/5p2/P2Q4/1P1R2R1/5P2/2K5 w - - 1 39 c0 2325089963416231556; c1 35305572812818; c2 2497; c3 0",
			"5k2/2pP4/5Np1/5p2/P2Q4/1P1R2R1/5P2/2K5 b - - 0 39 c0 2451161063215865477; c1 2206598300801; c2 156; c3 0",
			"5k2/2pP4/5Np1/8/P2Q1p2/1P1R2R1/5P2/2K5 w - - 0 40 c0 1154124370533162628; c1 2206598308424; c2 160; c3 0",
			"3Q1k2/2p5/5Np1/8/P2Q1p2/1P1R2R1/5P2/2K5 b - - 0 40 c0 1154124371469082245; c1 2206598308424; c2 160; c3 0",
			"3Q4/2p3k1/5Np1/8/P2Q1p2/1P1R2R1/5P2/2K5 w - - 1 41 c0 1154124372609932036; c1 1152923711205155400; c2 164; c3 0",
			"rn1qk2r/p4ppp/b1pbpn2/1p1pB3/3P4/P1N2N2/1PPQPPPP/R3KB1R b KQkq - 4 8 c0 6396836723146292745; c1 13907256767486040773; c2 14197325430213845088; c3 132217",
			"rn1q1rk1/p4ppp/b1pbpn2/1p1pB3/3P4/P1N2N2/1PPQPPPP/R3KB1R w KQ - 5 9 c0 6396836698450231048; c1 13907256767486040773; c2 14197325430213845088; c3 148761",
			"rn1q1rk1/p4ppp/b1pbpn2/1p1pB3/3P4/P1N1PN2/1PPQ1PPP/R3KB1R b KQ - 0 9 c0 6396836698450231049; c1 13907256767486040773; c2 14197325431081928208; c3 147481",
			"rn1q1rk1/p4ppp/b1p1pn2/1p1pb3/3P4/P1N1PN2/1PPQ1PPP/R3KB1R w KQ - 0 10 c0 6396836698450231048; c1 869203616687354181; c2 11263626380904243297; c3 10241",
			"rn1q1rk1/p4ppp/b1p1pn2/1p1pP3/8/P1N1PN2/1PPQ1PPP/R3KB1R b KQ - 0 10 c0 6396836698450231049; c1 1207246181724483909; c2 1856898153413362182; c3 640",
			"rn1qr1k1/p4ppp/b1p1pn2/1p1pP3/8/P1N1PN2/1PPQ1PPP/R3KB1R w KQ - 1 11 c0 6396836686102200072; c1 1207246181724483909; c2 1856898153413362182; c3 705",
			"rn1qr1k1/p4ppp/b1p1pP2/1p1p4/8/P1N1PN2/1PPQ1PPP/R3KB1R b KQ - 0 11 c0 6396836686102200073; c1 6992981915902947653; c2 116056134588335136; c3 44",
			"rn2r1k1/p4ppp/b1p1pq2/1p1p4/8/P1N1PN2/1PPQ1PPP/R3KB1R w KQ - 0 12 c0 6164409815894261512; c1 437061369743946516; c2 7253508411770946; c3 3",
			"rn2r1k1/p4ppp/b1p1pq2/1p1p4/3N4/P1N1P3/1PPQ1PPP/R3KB1R b KQ - 1 12 c0 6164409815894261513; c1 75452989168562964; c2 79311102449698882; c3 3",
			"rn2r1k1/p4p1p/b1p1pqp1/1p1p4/3N4/P1N1P3/1PPQ1PPP/R3KB1R w KQ - 0 13 c0 4996921981242638088; c1 75452989168568113; c2 4618939526839158850; c3 3",
			"rn2r1k1/p4p1p/b1p1pqp1/1p1p4/3NP3/P1N5/1PPQ1PPP/R3KB1R b KQ - 0 13 c0 4996921981242638089; c1 54325873240852273; c2 4618939526839158850; c3 3",
			"rn2r1k1/p4p1p/b1p2qp1/1p1pp3/3NP3/P1N5/1PPQ1PPP/R3KB1R w KQ - 0 14 c0 4996921981242638088; c1 54325873390265458; c2 9230625545266546754; c3 3",
			"rn2r1k1/p4p1p/b1p2qp1/1p1pp3/4P3/P1N2N2/1PPQ1PPP/R3KB1R b KQ - 1 14 c0 4996921981242638089; c1 435740925147882610; c2 9302683139304474690; c3 3",
			"rn2r1k1/p4p1p/b1p2qp1/1p2p3/3pP3/P1N2N2/1PPQ1PPP/R3KB1R w KQ - 0 15 c0 4996921981242638088; c1 435740961378804850; c2 13842311563693934658; c3 3",
			"rn2r1k1/p4p1p/b1p2qp1/1p2p3/3pP3/P4N2/1PPQNPPP/R3KB1R b KQ - 1 15 c0 4996921981242638089; c1 2332882304408626290; c2 13914369157731857540; c3 3",
			"rn2r1k1/p4p1p/b4qp1/1pp1p3/3pP3/P4N2/1PPQNPPP/R3KB1R w KQ - 0 16 c0 2402848595877232392; c1 2332882304408773191; c2 7253508411765892; c3 4",
			"rn2r1k1/p4p1p/b4qp1/1pp1p3/3pP3/P1P2N2/1P1QNPPP/R3KB1R b KQ - 0 16 c0 2402848595877232393; c1 433489161565266503; c2 7253508411765890; c3 4",
			"r3r1k1/p4p1p/b1n2qp1/1pp1p3/3pP3/P1P2N2/1P1QNPPP/R3KB1R w KQ - 1 17 c0 2654179430060195592; c1 433489161565266503; c2 4690997120877081730; c3 4",
			"r3r1k1/p4p1p/b1n2qp1/1pp1p3/1P1pP3/P1P2N2/3QNPPP/R3KB1R b KQ - 0 17 c0 2654179430060195593; c1 6935826585309438535; c2 4618939526839153792; c3 4",
			"r3r1k1/p4p1p/b1n2qp1/1p2p3/1PppP3/P1P2N2/3QNPPP/R3KB1R w KQ - 0 18 c0 2654179430060195592; c1 6935826603042742855; c2 9230625545266541696; c3 4",
			"r3r1k1/p4p1p/b1n2qp1/1p2p3/1PppP3/P1P2N2/3QNPPP/3RKB1R b K - 1 18 c0 2654179430060195593; c1 6935826603042742855; c2 9298179750130496640; c3 4",
			"r3r1k1/p4p1p/b1n2qp1/1p2p3/1Pp1P3/P1Pp1N2/3QNPPP/3RKB1R w K - 0 19 c0 2654179430060195592; c1 7531162156064121415; c2 13837808174519956608; c3 4",
			"r3r1k1/p4p1p/b1n2qp1/1p2p3/1Pp1P3/P1Pp1NN1/3Q1PPP/3RKB1R b K - 1 19 c0 2654179430060195593; c1 7531162156064121415; c2 13909865768557889548; c3 4",
			"r3r1k1/pb3p1p/2n2qp1/1p2p3/1Pp1P3/P1Pp1NN1/3Q1PPP/3RKB1R w K - 2 20 c0 2650546686591696648; c1 7531162156064121415; c2 146865307313653772; c3 5",
			"r3r1k1/pb3p1p/2n2qp1/1p2p3/1Pp1P2P/P1Pp1NN1/3Q1PP1/3RKB1R b K - 0 20 c0 2650546686591696649; c1 9818137545745179207; c2 2750119168737478; c3 5",
			"r3r1k1/pb3p2/2n2qp1/1p2p2p/1Pp1P2P/P1Pp1NN1/3Q1PP1/3RKB1R w K - 0 21 c0 8235995386949598984; c1 9818137545753039396; c2 4614436137596125382; c3 5",
			"r3r1k1/pb3p2/2n2qp1/1p2p1Qp/1Pp1P2P/P1Pp1NN1/5PP1/3RKB1R b K - 1 21 c0 8235995386949598985; c1 9516248142373524004; c2 4686493731633761384; c3 5",
			"r3r1k1/pb3pq1/2n3p1/1p2p1Qp/1Pp1P2P/P1Pp1NN1/5PP1/3RKB1R w K - 2 22 c0 4955756377107791624; c1 9516248142373524004; c2 9370237344099077224; c3 5",
			"r3r1k1/pb3pq1/2n3p1/1p2p1QN/1Pp1P2P/P1Pp1N2/5PP1/3RKB1R b K - 0 22 c0 4955756377107791625; c1 9516248142256083492; c2 6341240157785686120; c3 0",
			"r3r1k1/pb3pq1/2n5/1p2p1Qp/1Pp1P2P/P1Pp1N2/5PP1/3RKB1R w K - 0 23 c0 344070358680403720; c1 9818137545753121058; c2 414341908371087366; c3 0",
			"r3r1k1/pb3pq1/2n5/1p2p1Qp/1Pp1P2P/P1Pp4/3N1PP1/3RKB1R b K - 1 23 c0 344070358680403721; c1 9818137545753121058; c2 414623383347798400; c3 0",
			"r3r1k1/pb3p2/2n3q1/1p2p1Qp/1Pp1P2P/P1Pp4/3N1PP1/3RKB1R w K - 2 24 c0 7227189070418607880; c1 9818137545753121058; c2 432919256833991040; c3 0",
			"r3r1k1/pb3p2/2n3q1/1p2p1Qp/PPp1P2P/2Pp4/3N1PP1/3RKB1R b K - 0 24 c0 7227189070418607881; c1 9817989317288153378; c2 432356306880569728; c3 0",
			"r3r1k1/1b3p2/p1n3q1/1p2p1Qp/PPp1P2P/2Pp4/3N1PP1/3RKB1R w K - 0 25 c0 7227452916970487560; c1 9817989317288153378; c2 450370705390051712; c3 0",
			"r3r1k1/1b3p2/p1n3Q1/1p2p2p/PPp1P2P/2Pp4/3N1PP1/3RKB1R b K - 0 25 c0 2615766898543099657; c1 613624332330504482; c2 28148169086878232; c3 0",
			"r3r1k1/1b6/p1n3p1/1p2p2p/PPp1P2P/2Pp4/3N1PP1/3RKB1R w K - 0 26 c0 2613443493034655496; c1 9261723557625432338; c2 1829629312107553; c3 0",
			"r3r1k1/1b6/p1n3p1/1p2p2p/PPp1P2P/2Pp2P1/3N1P2/3RKB1R b K - 0 26 c0 2613443493034655497; c1 326581896922368274; c2 1829629312107032; c3 0",
			"r3r1k1/1b2n3/p5p1/1p2p2p/PPp1P2P/2Pp2P1/3N1P2/3RKB1R w K - 1 27 c0 2612388168030420744; c1 326581896922368274; c2 1901097567912472; c3 0",
			"r3r1k1/1b2n3/p5p1/1p2p2p/PPp1P2P/2Pp2P1/3N1P2/2R1KB1R b K - 2 27 c0 2612388168030420745; c1 326581896922368274; c2 1902197077705240; c3 0",
			"r1n1r1k1/1b6/p5p1/1p2p2p/PPp1P2P/2Pp2P1/3N1P2/2R1KB1R w K - 3 28 c0 2612387032208178952; c1 326581896922368274; c2 1973665333510680; c3 0",
			"r1n1r1k1/1b6/p5p1/Pp2p2p/1Pp1P2P/2Pp2P1/3N1P2/2R1KB1R b K - 0 28 c0 1459465527601331977; c1 326581896922829090; c2 1970366798627352; c3 0",
			"r1n1r1k1/8/p1b3p1/Pp2p2p/1Pp1P2P/2Pp2P1/3N1P2/2R1KB1R w K - 1 29 c0 1460662715375353608; c1 326581896922829090; c2 2041835054432792; c3 0",
			"r1n1r1k1/8/p1b3p1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N4/2R1KB1R b K - 0 29 c0 1460662715375353609; c1 4794152727274361122; c2 2040735542804864; c3 0",
			"r3r1k1/8/p1bn2p1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N4/2R1KB1R w K - 1 30 c0 1461933818803060488; c1 4794152727274361122; c2 2112203798610304; c3 0",
			"r3r1k1/8/p1bn2p1/Pp2p2p/1Pp1P2P/2Pp1PPB/3N4/2R1K2R b K - 2 30 c0 1461933818803060489; c1 4794152727274361122; c2 2113303253882920; c3 0",
			"3rr1k1/8/p1bn2p1/Pp2p2p/1Pp1P2P/2Pp1PPB/3N4/2R1K2R w K - 3 31 c0 1461933818804379400; c1 4794152727274361122; c2 2184771509688360; c3 0",
			"3rr1k1/8/p1bn2p1/Pp2p2p/1Pp1P2P/2Pp1PPB/3N4/1R2K2R b K - 4 31 c0 1461933818804379401; c1 4794152727274361122; c2 2185871006636072; c3 0",
			"3rr3/7k/p1bn2p1/Pp2p2p/1Pp1P2P/2Pp1PPB/3N4/1R2K2R w K - 5 32 c0 1461933818804378504; c1 4794152727274361122; c2 2257339262441512; c3 0",
			"3rr3/7k/p1bn2p1/Pp2p2p/1Pp1P2P/2Pp1PPB/3N4/3RK2R b K - 6 32 c0 1461933818804378505; c1 4794152727274361122; c2 2258438818109480; c3 0",
			"3rr3/3b3k/p2n2p1/Pp2p2p/1Pp1P2P/2Pp1PPB/3N4/3RK2R w K - 7 33 c0 1461859073488526216; c1 4794152727274361122; c2 2329907073914920; c3 0",
			"3rr3/3b3k/p2n2p1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N2B1/3RK2R b K - 8 33 c0 1461859073488526217; c1 4794152727274361122; c2 2331006585618816; c3 0",
			"3rr3/3b2k1/p2n2p1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N2B1/3RK2R w K - 9 34 c0 1461859073488526088; c1 4794152727274361122; c2 2402474841424256; c3 0",
			"3rr3/3b2k1/p2n2p1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N2B1/3R1RK1 b - - 10 34 c0 1461859073488526093; c1 4794152727274361122; c2 2403536235217280; c3 0",
			"3rr3/3b4/p2n1kp1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N2B1/3R1RK1 w - - 11 35 c0 1464551777464948364; c1 4794152727274361122; c2 2475004491022720; c3 0",
			"3rr3/3b4/p2n1kp1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N2B1/3RR1K1 b - - 12 35 c0 1464551777464948365; c1 4794152727274361122; c2 2476102123602304; c3 0",
			"3rr3/3b2k1/p2n2p1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N2B1/3RR1K1 w - - 13 36 c0 1461859073488526092; c1 4794152727274361122; c2 2547570379407744; c3 0",
			"3rr3/3b2k1/p2n2p1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N2B1/3RRK2 b - - 14 36 c0 1461859073488526091; c1 4794152727274361122; c2 2548669891035520; c3 0",
			"3rr3/3b4/p2n1kp1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N2B1/3RRK2 w - - 15 37 c0 1464551777464948362; c1 4794152727274361122; c2 2620138146840960; c3 0",
			"3rr3/3b4/p2n1kp1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N2B1/1R2RK2 b - - 16 37 c0 1464551777464948363; c1 4794152727274361122; c2 2621237614428544; c3 0",
			"3rr3/3b2k1/p2n2p1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N2B1/1R2RK2 w - - 17 38 c0 1461859073488526090; c1 4794152727274361122; c2 2692705870233984; c3 0",
			"3rr3/3b2k1/p2n2p1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N2B1/1R2R1K1 b - - 18 38 c0 1461859073488526093; c1 4794152727274361122; c2 2693805381861760; c3 0",
			"2r1r3/3b2k1/p2n2p1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N2B1/1R2R1K1 w - - 19 39 c0 1461859073487772428; c1 4794152727274361122; c2 2765273637667200; c3 0",
			"2r1r3/3b2k1/p2n2p1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N2B1/3RR1K1 b - - 20 39 c0 1461859073487772429; c1 4794152727274361122; c2 2766373193335168; c3 0",
			"1r2r3/3b2k1/p2n2p1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N2B1/3RR1K1 w - - 21 40 c0 1461859073487395596; c1 4794152727274361122; c2 2837841449140608; c3 0",
			"1r2r3/3b2k1/p2n2p1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N2B1/1R2R1K1 b - - 22 40 c0 1461859073487395597; c1 4794152727274361122; c2 2838940916728192; c3 0",
			"3rr3/3b2k1/p2n2p1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N2B1/1R2R1K1 w - - 23 41 c0 1461859073488526092; c1 4794152727274361122; c2 2910409172533632; c3 0",
			"3rr3/3b2k1/p2n2p1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/1R2R3 b - - 24 41 c0 1461859073488526107; c1 4794152727274361122; c2 2911507737256320; c3 0",
			"2r1r3/3b2k1/p2n2p1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/1R2R3 w - - 25 42 c0 1461859073487772442; c1 4794152727274361122; c2 2982975993061760; c3 0",
			"2r1r3/3b2k1/p2n2p1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/R3R3 b - - 26 42 c0 1461859073487772443; c1 4794152727274361122; c2 2984075501019520; c3 0",
			"2r1r3/6k1/p2nb1p1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/R3R3 w - - 27 43 c0 1465245547826486042; c1 4794152727274361122; c2 3055543756824960; c3 0",
			"2r1r3/6k1/p2nb1p1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/1R2R3 b - - 28 43 c0 1465245547826486043; c1 4794152727274361122; c2 3056643272122752; c3 0",
			"2r1r3/8/p2nbkp1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/1R2R3 w - - 29 44 c0 1471324816335935130; c1 4794152727274361122; c2 3128111527928192; c3 0",
			"2r1r3/8/p2nbkp1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/1R5R b - - 30 44 c0 1471324816335935131; c1 4794152727274361122; c2 3129217616224640; c3 0",
			"2r1r3/3b4/p2n1kp1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/1R5R w - - 31 45 c0 1464551777464194714; c1 4794152727274361122; c2 3200685872030080; c3 0",
			"2r1r3/3b4/p2n1kp1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/2R4R b - - 32 45 c0 1464551777464194715; c1 4794152727274361122; c2 3201785390997888; c3 0",
			"2r5/3br3/p2n1kp1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/2R4R w - - 33 46 c0 1464551890427287194; c1 4794152727274361122; c2 3273253646803328; c3 0",
			"2r5/3br3/p2n1kp1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/4R2R b - - 34 46 c0 1464551890427287195; c1 4794152727274361122; c2 3274353202471296; c3 0",
			"2r5/4r3/p2nbkp1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/4R2R w - - 35 47 c0 1471324828635731610; c1 4794152727274361122; c2 3345821458276736; c3 0",
			"2r5/4r3/p2nbkp1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/5R1R b - - 36 47 c0 1471324828635731611; c1 4794152727274361122; c2 3346921028624768; c3 0",
			"5r2/4r3/p2nbkp1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/5R1R w - - 37 48 c0 1471324828641007258; c1 4794152727274361122; c2 3418389284430208; c3 0",
			"5r2/4r3/p2nbkp1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/1R5R b - - 38 48 c0 1471324828641007259; c1 4794152727274361122; c2 3419488685957504; c3 0",
			"5r2/r7/p2nbkp1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/1R5R w - - 39 49 c0 1471324817064728218; c1 4794152727274361122; c2 3490956941762944; c3 0",
			"5r2/r7/p2nbkp1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/6RR b - - 40 49 c0 1471324817064728219; c1 4794152727274361122; c2 3492056680931712; c3 0",
			"5r2/5r2/p2nbkp1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/6RR w - - 41 50 c0 1471324840989038234; c1 4794152727274361122; c2 3563524936737152; c3 0",
			"5r2/5r2/p2nbkp1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/3R3R b - - 42 50 c0 1471324840989038235; c1 4794152727274361122; c2 3564624242844032; c3 0",
			"7r/5r2/p2nbkp1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/3R3R w - - 43 51 c0 1471324841007126170; c1 4794152727274361122; c2 3636092498649472; c3 0",
			"7r/5r2/p2nbkp1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/6RR b - - 44 51 c0 1471324841007126171; c1 4794152727274361122; c2 3637192215798144; c3 0",
			"2r5/5r2/p2nbkp1/Pp2p2p/1Pp1P2P/2Pp1PP1/3N1KB1/6RR w - - 45 52 c0 1471324840983762586; c1 4794152727274361122; c2 3708660471603584; c3 0",
			"2r5/5r2/p2nbkp1/Pp2p2p/1Pp1P2P/2PpKPP1/3N2B1/6RR b - - 46 52 c0 1471324840983762601; c1 2416252124022739234; c2 3709759983231168; c3 0",
			"r7/5r2/p2nbkp1/Pp2p2p/1Pp1P2P/2PpKPP1/3N2B1/6RR w - - 47 53 c0 1471324840983197352; c1 2416252124022739234; c2 3781228239036608; c3 0",
			"r7/5r2/p2nbkp1/Pp2p2p/1Pp1P2P/2PpKPP1/3N2B1/3R3R b - - 48 53 c0 1471324840983197353; c1 2416252124022739234; c2 3782327545143488; c3 0",
			"7r/5r2/p2nbkp1/Pp2p2p/1Pp1P2P/2PpKPP1/3N2B1/3R3R w - - 49 54 c0 1471324841007126184; c1 2416252124022739234; c2 3853795800948928; c3 0",
			"7r/5r2/p2nbkp1/Pp2p2p/1Pp1PP1P/2PpK1P1/3N2B1/3R3R b - - 0 54 c0 1471324841007126185; c1 2919467397413376290; c2 3799919731187904; c3 0",
			"7r/5r2/p2n1kp1/Pp2p2p/1Pp1PPbP/2PpK1P1/3N2B1/3R3R w - - 1 55 c0 2397377514385184424; c1 2919473035901831442; c2 3871387986993344; c3 0",
			"7r/5r2/p2n1kp1/Pp2p2p/1Pp1PPbP/2PpK1P1/3N2B1/5R1R b - - 2 55 c0 2397377514385184425; c1 2919473035901831442; c2 3872487586701504; c3 0",
			"5r2/5r2/p2n1kp1/Pp2p2p/1Pp1PPbP/2PpK1P1/3N2B1/5R1R w - - 3 56 c0 2397377514367096488; c1 2919473035901831442; c2 3943955842506944; c3 0",
			"5r2/5r2/p2n1kp1/Pp2p2p/1Pp1PPbP/2PpKBP1/3N4/5R1R b - - 4 56 c0 2397377514367096489; c1 6378237549722372370; c2 3945055354096642; c3 0",
			"5r2/5r2/p2n1kp1/Pp2p2p/1Pp1PP1P/2PpKbP1/3N4/5R1R w - - 0 57 c0 2397377514367096488; c1 3857404008272660754; c2 250689128235200; c3 0",
			"5r2/5r2/p2n1kp1/Pp2p2p/1Pp1PP1P/2PpKNP1/8/5R1R b - - 0 57 c0 2397377514367096489; c1 2560367315589957906; c2 15668070514688; c3 0",
			"5r2/5r2/p2n1kp1/Pp5p/1Pp1Pp1P/2PpKNP1/8/5R1R w - - 0 58 c0 2397377514367096488; c1 160022965814306946; c2 996434276352; c3 0",
			"5r2/5r2/p2n1kp1/Pp5p/1Pp1PP1P/2PpKN2/8/5R1R b - - 0 58 c0 2397377514367096489; c1 15907769148516482; c2 62277142272; c3 0",
			"5r2/4kr2/p2n2p1/Pp5p/1Pp1PP1P/2PpKN2/8/5R1R w - - 1 59 c0 2397209208020539944; c1 15907769148516482; c2 63367661312; c3 0",
			"5r2/4kr2/p2n2p1/Pp2N2p/1Pp1PP1P/2PpK3/8/5R1R b - - 2 59 c0 2397209208020539945; c1 38351524262479922; c2 63384438528; c3 0",
			"5r2/4k1r1/p2n2p1/Pp2N2p/1Pp1PP1P/2PpK3/8/5R1R w - - 3 60 c0 2397209220368570920; c1 38351524262479922; c2 64474957568; c3 0",
			"5r2/4k1r1/p2n2p1/Pp2N2p/1Pp1PP1P/2PpK3/8/5RR1 b - - 4 60 c0 2397209220368570921; c1 38351524262479922; c2 64491677440; c3 0",
			"8/4k1r1/p2n1rp1/Pp2N2p/1Pp1PP1P/2PpK3/8/5RR1 w - - 5 61 c0 2397860662144408104; c1 38351524262479922; c2 65582196480; c3 0",
			"8/4k1r1/p2n1rp1/Pp2NP1p/1Pp1P2P/2PpK3/8/5RR1 b - - 0 61 c0 2397860662144408105; c1 38351520771113522; c2 65498310400; c3 0",
			"3k4/6r1/p2n1rp1/Pp2NP1p/1Pp1P2P/2PpK3/8/5RR1 w - - 1 62 c0 2397860662144409000; c1 38351520771113522; c2 66588829440; c3 0",
			"3k4/6r1/p2n1rP1/Pp2N2p/1Pp1P2P/2PpK3/8/5RR1 b - - 0 62 c0 2379846263634927017; c1 2396970048194610; c2 4160753264; c3 0",
			"3k4/6r1/p2n2P1/Pp2N2p/1Pp1P2P/2PpK3/8/5rR1 w - - 0 63 c0 2454542685579517352; c1 149810628012163; c2 264241399; c3 0",
			"3k4/6r1/p2n2P1/Pp2N2p/1Pp1P2P/2PpK3/8/5R2 b - - 0 63 c0 2454542685579517353; c1 149810628012163; c2 16515079; c3 0",
			"8/4k1r1/p2n2P1/Pp2N2p/1Pp1P2P/2PpK3/8/5R2 w - - 1 64 c0 2454542685579516456; c1 149810628012163; c2 16781319; c3 0",
			"8/4k1r1/p1Nn2P1/Pp5p/1Pp1P2P/2PpK3/8/5R2 b - - 2 64 c0 2379194528117561897; c1 149810628012162; c2 16785415; c3 0",
			"8/6r1/p1Nnk1P1/Pp5p/1Pp1P2P/2PpK3/8/5R2 w - - 3 65 c0 2379362553076651560; c1 149810628012162; c2 17051655; c3 0",
			"8/6r1/p1Nnk1P1/Pp5p/1Pp1P2P/2PpK3/8/6R1 b - - 4 65 c0 2379362553076651561; c1 149810628012162; c2 17055758; c3 0",
			"2n5/6r1/p1N1k1P1/Pp5p/1Pp1P2P/2PpK3/8/6R1 w - - 5 66 c0 2379055277292492328; c1 149810628012162; c2 17321998; c3 0",
			"2n5/6r1/p3k1P1/Pp5p/1PpNP2P/2PpK3/8/6R1 b - - 6 66 c0 2454534551516059177; c1 149810625397384; c2 17326094; c3 0",
			"2n5/4k1r1/p5P1/Pp5p/1PpNP2P/2PpK3/8/6R1 w - - 7 67 c0 2454533358588893736; c1 149810625397384; c2 17592334; c3 0",
			"2n5/4k1r1/p5P1/Pp5p/1PpNP2P/2PpK1R1/8/8 b - - 8 67 c0 2454533358588893737; c1 4090460299346568; c2 17596416; c3 0",
			"2n1k3/6r1/p5P1/Pp5p/1PpNP2P/2PpK1R1/8/8 w - - 9 68 c0 2454533358588894760; c1 4090460299346568; c2 17862656; c3 0",
			"2n1k3/6r1/p3N1P1/Pp5p/1Pp1P2P/2PpK1R1/8/8 b - - 10 68 c0 2379080472644394537; c1 4090460301961346; c2 17866752; c3 0",
			"2n1k1r1/8/p3N1P1/Pp5p/1Pp1P2P/2PpK1R1/8/8 w - - 11 69 c0 2379080448044801576; c1 4090460301961346; c2 18132992; c3 0",
			"2n1k1r1/8/p3N1P1/Pp4Rp/1Pp1P2P/2PpK3/8/8 b - - 12 69 c0 2379080448044801577; c1 2396970048195010; c2 18137088; c3 0",
			"2n3r1/4k3/p3N1P1/Pp4Rp/1Pp1P2P/2PpK3/8/8 w - - 13 70 c0 2379080448141269544; c1 2396970048195010; c2 18403328; c3 0",
			"2n3r1/4k3/p5P1/PpN3Rp/1Pp1P2P/2PpK3/8/8 b - - 14 70 c0 16289591389367933481; c1 2396970048195008; c2 18407424; c3 0",
			"2n2kr1/8/p5P1/PpN3Rp/1Pp1P2P/2PpK3/8/8 w - - 15 71 c0 16289591389271465640; c1 2396970048195008; c2 18673664; c3 0",
			"2n2kr1/8/N5P1/Pp4Rp/1Pp1P2P/2PpK3/8/8 b - - 0 71 c0 2454532371916627625; c1 149810628012188; c2 1163264; c3 0",
			"5kr1/4n3/N5P1/Pp4Rp/1Pp1P2P/2PpK3/8/8 w - - 1 72 c0 2454532376925839016; c1 149810628012188; c2 1179904; c3 0",
			"5kr1/2N1n3/6P1/Pp4Rp/1Pp1P2P/2PpK3/8/8 b - - 2 72 c0 2454532247472840361; c1 149810628012188; c2 1180160; c3 0",
			"5kr1/2N5/6n1/Pp4Rp/1Pp1P2P/2PpK3/8/8 w - - 0 73 c0 13988545480680545960; c1 9363164250761; c2 74752; c3 0",
			"5kr1/2N5/P5n1/1p4Rp/1Pp1P2P/2PpK3/8/8 b - - 0 73 c0 13989517517678976681; c1 9363164250761; c2 74752; c3 0",
			"5kr1/2N5/P5n1/1p4Rp/1Pp1P2P/2P1K3/3p4/8 w - - 0 74 c0 13989517517678976680; c1 1196285966434953; c2 75776; c3 0",
			"5kr1/2N5/P5n1/1p4Rp/1Pp1P2P/2P5/3K4/8 b - - 0 74 c0 13989517517678976663; c1 17315414665; c2 4736; c3 0",
			"6r1/2N1k3/P5n1/1p4Rp/1Pp1P2P/2P5/3K4/8 w - - 1 75 c0 13989517517886331414; c1 17315414665; c2 4801; c3 0",
			"6r1/2N1k3/P5n1/1p4Rp/1Pp1P2P/2P1K3/8/8 b - - 2 75 c0 13989517517886331433; c1 17315414665; c2 4802; c3 0",
			"6r1/2N5/P2k2n1/1p4Rp/1Pp1P2P/2P1K3/8/8 w - - 3 76 c0 13989517586605807016; c1 17315414665; c2 4867; c3 0",
			"6r1/2N5/P2k2n1/1pR4p/1Pp1P2P/2P1K3/8/8 b - - 4 76 c0 2172072164385625513; c1 17315414664; c2 4868; c3 0",
			"6r1/2Nk4/P5n1/1pR4p/1Pp1P2P/2P1K3/8/8 w - - 5 77 c0 2172072095666149800; c1 17315414664; c2 4933; c3 0",
			"6r1/P1Nk4/6n1/1pR4p/1Pp1P2P/2P1K3/8/8 b - - 0 77 c0 2172072033020025257; c1 17315414664; c2 4928; c3 0",
			"6r1/P1Nk4/8/1pR4p/1Pp1P2n/2P1K3/8/8 w - - 0 78 c0 9359042982117382568; c1 1233208360; c2 312; c3 0",
			"Q5r1/2Nk4/8/1pR4p/1Pp1P2n/2P1K3/8/8 b - - 0 78 c0 9359042982264781225; c1 1233208360; c2 312; c3 0",
			"Q5r1/2Nk4/8/1pR4p/1Pp1P3/2P1K3/6n1/8 w - - 1 79 c0 9359042982264781224; c1 1152963286115894312; c2 316; c3 0",
			"Q5r1/2Nk4/8/1pR4p/1Pp1P3/2P5/5Kn1/8 b - - 2 79 c0 9359042982264781211; c1 2305884790722741288; c2 316; c3 0",
			"r7/2Nk4/8/1pR4p/1Pp1P3/2P5/5Kn1/8 w - - 0 80 c0 9808312223234455962; c1 2611344315458; c2 20; c3 0",
			"N7/3k4/8/1pR4p/1Pp1P3/2P5/5Kn1/8 b - - 0 80 c0 2918862523140700571; c1 4611686181636407620; c2 1; c3 0",
			"N7/3k4/8/1pR4p/1Pp1Pn2/2P5/5K2/8 w - - 1 81 c0 2918862523140700570; c1 4904419994210820420; c2 1; c3 0",
			"N7/3k4/8/1pR4p/1Pp1Pn2/2P1K3/8/8 b - - 2 81 c0 2918862523140700585; c1 4908923593838190916; c2 1; c3 0",
			"N7/3k4/6n1/1pR4p/1Pp1P3/2P1K3/8/8 w - - 3 82 c0 9808317445511870888; c1 5201657569617122370; c2 1; c3 0",
			"N7/3k4/6n1/1pR4p/1PpKP3/2P5/8/8 b - - 4 82 c0 9808317445511870903; c1 5206161169242393666; c2 1; c3 0",
			"N7/3k4/6n1/1pR5/1PpKP2p/2P5/8/8 w - - 0 83 c0 2314327665567365558; c1 5476377146884899012; c2 1; c3 0",
			"N7/3k4/6n1/1p4R1/1PpKP2p/2P5/8/8 b - - 1 83 c0 2432547155785841079; c1 5480880746512269508; c2 1; c3 0",
			"N4n2/3k4/8/1p4R1/1PpKP2p/2P5/8/8 w - - 2 84 c0 2432541933185300918; c1 5773614722291351748; c2 1; c3 0",
			"N4n2/3k4/8/1R6/1PpKP2p/2P5/8/8 b - - 0 84 c0 4756047497187588535; c1 1513209474796635148; c2 0; c3 0",
			"N7/3k4/4n3/1R6/1PpKP2p/2P5/8/8 w - - 1 85 c0 4756048802777954742; c1 1531505348282827788; c2 0; c3 0",
			"N7/3k4/4n3/1R1K4/1Pp1P2p/2P5/8/8 b - - 2 85 c0 2378148199526332871; c1 1531786823259538442; c2 0; c3 0",
			"N7/3k4/8/1R1K4/1Pp1Pn1p/2P5/8/8 w - - 3 86 c0 11677849226934516166; c1 1550082696745731376; c2 0; c3 0",
			"N7/3k4/8/1R6/1PK1Pn1p/2P5/8/8 b - - 0 86 c0 585483344720984501; c1 96827391988474963; c2 0; c3 0",
			"N7/3k4/6n1/1R6/1PK1P2p/2P5/8/8 w - - 1 87 c0 9367738738215516596; c1 97970884081361984; c2 0; c3 0",
			"N7/3k4/6n1/1R2P3/1PK4p/2P5/8/8 b - - 0 87 c0 2310598122125949365; c1 97953291895317568; c2 0; c3 0",
			"N7/3k4/6n1/1R2P3/1PK5/2P4p/8/8 w - - 0 88 c0 2310598122125949364; c1 99079191806607872; c2 0; c3 0",
			"N7/3k4/6n1/2R1P3/1PK5/2P4p/8/8 b - - 1 88 c0 2310844412730571189; c1 99096783992652288; c2 0; c3 0",
			"N7/3k4/6n1/2R1P3/1PK5/2P5/7p/8 w - - 0 89 c0 2310844412730571188; c1 100205092849844736; c2 0; c3 0",
			"N7/2Rk4/6n1/4P3/1PK5/2P5/7p/8 b - - 1 89 c0 2310430172664330677; c1 100222685035889152; c2 0; c3 0",
			"N7/2R5/4k1n1/4P3/1PK5/2P5/7p/8 w - - 2 90 c0 2310430172664329780; c1 101366177128776192; c2 0; c3 0",
			"N7/7R/4k1n1/4P3/1PK5/2P5/7p/8 b - - 3 90 c0 2310430201789576757; c1 101383769314820608; c2 0; c3 0",
			"N7/7R/4k3/4n3/1PK5/2P5/7p/8 w - - 0 91 c0 149463242698159668; c1 6403555791470624; c2 0; c3 0",
			"N7/7R/4k3/2K1n3/1P6/2P5/7p/8 b - - 1 91 c0 74731636381480517; c1 6404655303098400; c2 0; c3 0",
			"N7/7R/4k3/2K5/1P6/2P2n2/7p/8 w - - 2 92 c0 4503629692171844; c1 6476123558908674; c2 0; c3 0",
			"8/2N4R/4k3/2K5/1P6/2P2n2/7p/8 b - - 3 92 c0 4503629717313093; c1 6477223070536450; c2 0; c3 0",
			"8/2N4R/8/2K1k3/1P6/2P2n2/7p/8 w - - 4 93 c0 4503629717312068; c1 6548691326341890; c2 0; c3 0",
			"8/7R/8/2KNk3/1P6/2P2n2/7p/8 b - - 5 93 c0 4529989785489989; c1 6549790837969666; c2 0; c3 0",
			"8/7R/8/2KN4/1P2k3/2P2n2/7p/8 w - - 6 94 c0 9033589412859460; c1 6621259093775106; c2 0; c3 0",
			"8/7R/8/2KN4/1PP1k3/5n2/7p/8 b - - 0 94 c0 297263965564571205; c1 6614662024008448; c2 0; c3 0",
			"8/7R/8/2KNn3/1PP1k3/8/7p/8 w - - 1 95 c0 4761175621218864708; c1 6686130279809024; c2 0; c3 0",
			"8/8/8/2KNn3/1PP1k3/8/7R/8 b - - 0 95 c0 297573476208741957; c1 417814420389888; c2 0; c3 0",
			"8/8/8/2KN4/1PP1k3/5n2/7R/8 w - - 1 96 c0 18578997730348612; c1 422281186378032; c2 0; c3 0",
			"8/8/8/2KN4/1PP1k3/5n2/4R3/8 b - - 2 96 c0 18578997730348613; c1 422349904249136; c2 0; c3 0",
			"8/8/8/2KN1k2/1PP5/5n2/4R3/8 w - - 3 97 c0 9290323498898116; c1 426816670236976; c2 0; c3 0",
			"8/8/8/2KN1k2/1PP5/5n2/5R2/8 b - - 4 97 c0 9290323498898117; c1 426885389943088; c2 0; c3 0",
			"8/8/8/2KN4/1PP1k3/5n2/5R2/8 w - - 5 98 c0 18578997730348612; c1 431352155930928; c2 0; c3 0",
			"8/8/8/2KN4/1PP1k3/5R2/8/8 b - - 0 98 c0 18578997730348613; c1 26938034880624; c2 0; c3 0",
			"8/8/8/2KN4/1PP5/5k2/8/8 w - - 0 99 c0 18578997730347716; c1 1700807049216; c2 0; c3 0",
			"8/8/8/1PKN4/2P5/5k2/8/8 b - - 0 99 c0 18041061666458309; c1 1700807049216; c2 0; c3 0",
			"8/8/8/1PKN4/2P5/8/5k2/8 w - - 1 100 c0 18041061666457284; c1 1718255353856; c2 0; c3 0",
			"8/8/1P6/2KN4/2P5/8/5k2/8 b - - 0 100 c0 18040787862292165; c1 1717986918400; c2 0; c3 0",
			"8/8/1P6/2KN4/2P5/6k1/8/8 w - - 1 101 c0 18040787862293316; c1 1735435223040; c2 0; c3 0",
			"8/1P6/8/2KN4/2P5/6k1/8/8 b - - 0 101 c0 18040786792745797; c1 1735166787584; c2 0; c3 0",
			"8/1P6/8/2KN4/2P4k/8/8/8 w - - 1 102 c0 18040786792746948; c1 1752615092224; c2 0; c3 0",
			"1Q6/8/8/2KN4/2P4k/8/8/8 b - - 0 102 c0 18040786788700101; c1 1752346656768; c2 0; c3 0",
			"1Q6/8/8/2KN2k1/2P5/8/8/8 w - - 1 103 c0 9033587533960004; c1 1769794961408; c2 0; c3 0",
			"8/8/8/2KN2k1/2P2Q2/8/8/8 b - - 2 103 c0 649082945562219333; c1 1770063396864; c2 0; c3 0",
			"8/8/6k1/2KN4/2P2Q2/8/8/8 w - - 3 104 c0 649082120928499524; c1 1787511701504; c2 0; c3 0",
			"8/8/3K2k1/3N4/2P2Q2/8/8/8 b - - 4 104 c0 649082120928499543; c1 1787780136960; c2 0; c3 0",
			"8/7k/3K4/3N4/2P2Q2/8/8/8 w - - 5 105 c0 649082120928500694; c1 1805228441600; c2 0; c3 0",
			"8/7k/3K4/3N4/2P4Q/8/8/8 b - - 6 105 c0 2594637159952554967; c1 1805496877056; c2 0; c3 0",
			"8/8/3K2k1/3N4/2P4Q/8/8/8 w - - 7 106 c0 2594637159952553814; c1 1822945181696; c2 0; c3 0",
			"8/4N3/3K2k1/8/2P4Q/8/8/8 b - - 8 106 c0 2594636335419496279; c1 1823213617152; c2 0; c3 0",
			"8/4N1k1/3K4/8/2P4Q/8/8/8 w - - 9 107 c0 2594636335419497302; c1 1840661921792; c2 0; c3 0",
			"8/4N1k1/3K4/6Q1/2P5/8/8/8 b - - 10 107 c0 9323858704210775; c1 1840930357248; c2 0; c3 0",
			"8/4Nk2/3K4/6Q1/2P5/8/8/8 w - - 11 108 c0 9323858704210646; c1 1858378661888; c2 0; c3 0",
			"r1bqkb1r/2pn1ppp/4pn2/p2p4/QP1P4/P1P1P3/5PPP/R1B1KBNR b KQkq - 0 8 c0 14279160399812820489; c1 4683887169751904520; c2 8774790289501061136; c3 512",
			"r2qkb1r/1bpn1ppp/4pn2/p2p4/QP1P4/P1P1P3/5PPP/R1B1KBNR w KQkq - 1 9 c0 14279162534175112712; c1 4683887169751904520; c2 8774790289501061136; c3 577",
			"r2qkb1r/1bpn1ppp/4pn2/P2p4/Q2P4/P1P1P3/5PPP/R1B1KBNR b KQkq - 0 9 c0 14279162534175112713; c1 292743392630305032; c2 548424393093816321; c3 36",
			"r2qkb1r/1bpn1ppp/4p3/P2p4/Q2Pn3/P1P1P3/5PPP/R1B1KBNR w KQkq - 1 10 c0 14279162534175112712; c1 293068815867121928; c2 1701345897700663297; c3 40",
			"r2qkb1r/1bpn1ppp/P3p3/3p4/Q2Pn3/P1P1P3/5PPP/R1B1KBNR b KQkq - 0 10 c0 14279162534175112713; c1 293068815867121688; c2 548424393093816321; c3 40",
			"r2qkb1r/1b1n1ppp/P1p1p3/3p4/Q2Pn3/P1P1P3/5PPP/R1B1KBNR w KQkq - 0 11 c0 10115817775136177672; c1 293068815867122753; c2 548424393093816321; c3 44",
			"r2qkb1r/1b1n1ppp/P1p1p3/3p4/3Pn3/P1P1P3/2Q2PPP/R1B1KBNR b KQkq - 1 11 c0 10115817775136177673; c1 1171238303316382785; c2 1701345897700663872; c3 44",
			"r2qkb1r/3n1ppp/b1p1p3/3p4/3Pn3/P1P1P3/2Q2PPP/R1B1KBNR w KQkq - 0 12 c0 6396846000582098440; c1 73202393957273925; c2 34276524568363556; c3 3",
			"r2qkb1r/3n1ppp/B1p1p3/3p4/3Pn3/P1P1P3/2Q2PPP/R1B1K1NR b KQkq - 0 12 c0 6396846000582098441; c1 73202393957273924; c2 3460906780274999844; c3 0",
			"3qkb1r/3n1ppp/r1p1p3/3p4/3Pn3/P1P1P3/2Q2PPP/R1B1K1NR w KQk - 0 13 c0 6308525586146467336; c1 4616261168049717524; c2 234250703532491810; c3 0",
			"3qkb1r/3n1ppp/r1p1p3/3p4/3Pn2P/P1P1P3/2Q2PP1/R1B1K1NR b KQk - 0 13 c0 6308525586146467337; c1 73201717625782548; c2 234250703532474916; c3 0",
			"3qk2r/3n1ppp/r1pbp3/3p4/3Pn2P/P1P1P3/2Q2PP1/R1B1K1NR w KQk - 1 14 c0 14229340904415108616; c1 73201717625782554; c2 252546577018667556; c3 0",
			"3qk2r/3n1ppp/r1pbp3/3p4/P2Pn2P/2P1P3/2Q2PP1/R1B1K1NR b KQk - 0 14 c0 14229340904415108617; c1 73193597872668954; c2 252265102041956900; c3 0",
			"3qk2r/3n1ppp/r2bp3/2pp4/P2Pn2P/2P1P3/2Q2PP1/R1B1K1NR w KQk - 0 15 c0 12211728271353126408; c1 73193597872686097; c2 270279500551438884; c3 0",
			"3qk2r/3n1ppp/r2bp3/2pp4/P2Pn2P/B1P1P3/2Q2PP1/R3K1NR b KQk - 1 15 c0 12211728271353126409; c1 1172353381927797777; c2 270560975302959680; c3 0",
			"3qk2r/5ppp/rn1bp3/2pp4/P2Pn2P/B1P1P3/2Q2PP1/R3K1NR w KQk - 2 16 c0 12283440861405584904; c1 1172353381927797777; c2 288856848789152320; c3 0",
			"3qk2r/5ppp/rn1bp3/2pp4/P2Pn2P/B1P1PN2/2Q2PP1/R3K2R b KQk - 3 16 c0 12283440861405584905; c1 1172353381927797777; c2 289138299761730566; c3 0",
			"3qk2r/5ppp/1n1bp3/2pp4/r2Pn2P/B1P1PN2/2Q2PP1/R3K2R w KQk - 0 17 c0 1920376643849362952; c1 6990801114034637889; c2 19144267083817536; c3 0",
			"3qk2r/5ppp/1n1bp3/2pp4/r2Pn2P/B1P1PN2/2Q2PP1/R4RK1 b k - 1 17 c0 1920376643849362957; c1 6990801114034637889; c2 19160149872878144; c3 0",
			"3qk2r/5ppp/1n1bp3/2pp4/2rPn2P/B1P1PN2/2Q2PP1/R4RK1 w k - 2 18 c0 1920376643849362956; c1 6990801114106989633; c2 20303641965765184; c3 0",
			"3qk2r/5ppp/1n1bp3/2pp4/2rPn2P/B1P1PN2/4QPP1/R4RK1 b k - 3 18 c0 1920376643849362957; c1 6990801114106989633; c2 20321234151811328; c3 0",
			"3q1rk1/5ppp/1n1bp3/2pp4/2rPn2P/B1P1PN2/4QPP1/R4RK1 w - - 4 19 c0 1920376643752894220; c1 6990801114106989633; c2 21462527221442816; c3 0",
			"3q1rk1/5ppp/1n1bp3/2pp4/2rPn2P/B1P1PN2/5PP1/R3QRK1 b - - 5 19 c0 1920376643752894221; c1 6990801114106989633; c2 21480121713050112; c3 0",
			"3q1rk1/5ppp/3bp3/2pp4/n1rPn2P/B1P1PN2/5PP1/R3QRK1 w - - 6 20 c0 1272421247364964108; c1 6990801114108201156; c2 22623613805937152; c3 0",
			"3q1rk1/5ppp/3bp3/2Bp4/n1rPn2P/2P1PN2/5PP1/R3QRK1 b - - 0 20 c0 1272421247364964109; c1 436920164191242388; c2 1407378793104416; c3 0",
			"3q1rk1/5ppp/3bp3/2np4/2rPn2P/2P1PN2/5PP1/R3QRK1 w - - 0 21 c0 1272421247364964108; c1 27307510261876940; c2 92359221080130; c3 0",
			"3q1rk1/5ppp/3bp3/2Pp4/2r1n2P/2P1PN2/5PP1/R3QRK1 b - - 0 21 c0 1272421247364964109; c1 2307549728610191492; c2 5772451317508; c3 0",
			"3q1rk1/5ppp/3bp3/2np4/2r4P/2P1PN2/5PP1/R3QRK1 w - - 0 22 c0 1272421247364964108; c1 4755907876451256524; c2 377958076528; c3 0",
			"3q1rk1/5ppp/3bp3/2np4/2r4P/2P1PN2/4QPP1/R4RK1 b - - 1 22 c0 1272421247364964109; c1 2954468025503058124; c2 378226476804; c3 0",
			"5rk1/5ppp/3bpq2/2np4/2r4P/2P1PN2/4QPP1/R4RK1 w - - 2 23 c0 3682406029856612108; c1 2954468025503058124; c2 395674781444; c3 0",
			"5rk1/5ppp/3bpq2/2np4/2r4P/2P1PN2/4QPP1/R1R3K1 b - - 3 23 c0 3682406029856612109; c1 2954468025503058124; c2 395942414084; c3 0",
			"5rk1/5ppp/3bpq2/3p4/2r4P/1nP1PN2/4QPP1/R1R3K1 w - - 4 24 c0 3682406029856612108; c1 2954468035572121736; c2 413390718724; c3 0",
			"5rk1/5ppp/3bpq2/3p4/2r4P/1nP1PN2/Q4PP1/R1R3K1 b - - 5 24 c0 3682406029856612109; c1 2346482085877104776; c2 413659154180; c3 0",
			"5rk1/5ppp/3bpq2/3p4/2r4P/2P1PN2/Q4PP1/R1n3K1 w - - 0 25 c0 3682406029856612108; c1 4758341148165390472; c2 26843565168; c3 0",
			"5rk1/5ppp/3bpq2/3p4/2r4P/2P1PN2/Q4PP1/2R3K1 b - - 0 25 c0 3682406029856612109; c1 4758341148165390472; c2 1677722048; c3 0",
			"5rk1/5ppp/3b1q2/3pp3/2r4P/2P1PN2/Q4PP1/2R3K1 w - - 0 26 c0 9451517152518217484; c1 4758341148165390616; c2 1744830912; c3 0",
			"5rk1/5ppp/3b1q2/3pp3/2r4P/2P1PN2/2Q2PP1/2R3K1 b - - 1 26 c0 9451517152518217485; c1 4765940972536578328; c2 1745879488; c3 0",
			"5rk1/5ppp/3b1q2/3p4/2r1p2P/2P1PN2/2Q2PP1/2R3K1 w - - 0 27 c0 9451517152518217484; c1 4765940972537338888; c2 1811939776; c3 0",
			"5rk1/5ppp/3b1q2/3p4/2r1p2P/2P1P3/2QN1PP1/2R3K1 b - - 1 27 c0 9451517152518217485; c1 4763189994444643336; c2 1812988352; c3 0",
			"2r2rk1/5ppp/3b1q2/3p4/4p2P/2P1P3/2QN1PP1/2R3K1 w - - 2 28 c0 3650321850615701260; c1 4763189994444619912; c2 1881145792; c3 0",
			"2r2rk1/5ppp/3b1q2/3p4/4p2P/2P1P1P1/2QN1P2/2R3K1 b - - 0 28 c0 3650321850615701261; c1 2424066967281401992; c2 1879048640; c3 0",
			"2r2rk1/5pp1/3b1q2/3p3p/4p2P/2P1P1P1/2QN1P2/2R3K1 w - - 0 29 c0 9451512324529299212; c1 2424066967281404040; c2 1946157504; c3 0",
			"2r2rk1/5pp1/3b1q2/3p3p/4p2P/2P1P1P1/1Q1N1P2/2R3K1 b - - 1 29 c0 9451512324529299213; c1 2419000417700612232; c2 1947206080; c3 0",
			"1r3rk1/5pp1/3b1q2/3p3p/4p2P/2P1P1P1/1Q1N1P2/2R3K1 w - - 2 30 c0 9451512324528922380; c1 2419000417700612232; c2 2015363520; c3 0",
			"1r3rk1/5pp1/3b1q2/3p3p/4p2P/2P1P1P1/3N1P2/Q1R3K1 b - - 3 30 c0 9451512324528922381; c1 150875055366146184; c2 2016412105; c3 0",
			"r4rk1/5pp1/3b1q2/3p3p/4p2P/2P1P1P1/3N1P2/Q1R3K1 w - - 4 31 c0 9451512324528733964; c1 150875055366146184; c2 2084569545; c3 0",
			"r4rk1/5pp1/3b1q2/3p3p/4p2P/2P1P1P1/3N1P2/1QR3K1 b - - 5 31 c0 9451512324528733965; c1 150875055366146184; c2 2085618130; c3 0",
			"rr4k1/5pp1/3b1q2/3p3p/4p2P/2P1P1P1/3N1P2/1QR3K1 w - - 6 32 c0 9451512324438294284; c1 150875055366146184; c2 2153775570; c3 0",
			"rr4k1/5pp1/3b1q2/3p3p/4p2P/2P1P1P1/2QN1P2/2R3K1 b - - 7 32 c0 9451512324438294285; c1 2424066967281404040; c2 2154824128; c3 0",
			"rr4k1/5pp1/5q2/3pb2p/4p2P/2P1P1P1/2QN1P2/2R3K1 w - - 8 33 c0 9813911356953263884; c1 2424066967281404053; c2 2222981568; c3 0",
			"rr4k1/5pp1/5q2/3pb2p/4p2P/1NP1P1P1/2Q2P2/2R3K1 b - - 9 33 c0 9813911356953263885; c1 2468044082854824085; c2 2224030144; c3 0",
			"r1r3k1/5pp1/5q2/3pb2p/4p2P/1NP1P1P1/2Q2P2/2R3K1 w - - 10 34 c0 9813911356959293196; c1 2468044082854824085; c2 2292187584; c3 0",
			"r1r3k1/5pp1/5q2/3pb2p/4p2P/1NP1P1P1/3Q1P2/2R3K1 b - - 11 34 c0 9813911356959293197; c1 2630173669440161941; c2 2293236160; c3 0",
			"2r3k1/5pp1/5q2/3pb2p/4p2P/rNP1P1P1/3Q1P2/2R3K1 w - - 12 35 c0 6377976982844186380; c1 2630173669817127049; c2 2361393600; c3 0",
			"2r3k1/5pp1/5q2/3pb2p/3Np2P/r1P1P1P1/3Q1P2/2R3K1 b - - 13 35 c0 6377976982844186381; c1 2630173674003568777; c2 2362442176; c3 0",
			"2r3k1/5pp1/5q2/3p3p/3bp2P/r1P1P1P1/3Q1P2/2R3K1 w - - 0 36 c0 9836741496664727308; c1 164385854625225352; c2 150994972; c3 0",
			"2r3k1/5pp1/5q2/3p3p/3Pp2P/r1P3P1/3Q1P2/2R3K1 b - - 0 36 c0 9836741496664727309; c1 13845332168270221448; c2 9437185; c3 0",
			"r5k1/5pp1/5q2/3p3p/3Pp2P/r1P3P1/3Q1P2/2R3K1 w - - 1 37 c0 9836741496664162060; c1 13845332168270221448; c2 9703425; c3 0",
			"r5k1/5pp1/5q2/3p3p/3PpQ1P/r1P3P1/5P2/2R3K1 b - - 2 37 c0 9836741496664162061; c1 13844069675946938504; c2 9707521; c3 0",
			"r5k1/5pp1/5q2/3p3p/3PpQ1P/2P3P1/5P2/r1R3K1 w - - 3 38 c0 9836741496664162060; c1 15492945944069214344; c2 9973761; c3 0",
			"rQ4k1/5pp1/5q2/3p3p/3Pp2P/2P3P1/5P2/r1R3K1 b - - 4 38 c0 9813911356949593869; c1 15492945944069081224; c2 9977857; c3 0",
			"rQ6/5ppk/5q2/3p3p/3Pp2P/2P3P1/5P2/r1R3K1 w - - 5 39 c0 9813916175902899084; c1 15492945944069081224; c2 10244097; c3 0",
			"r7/5ppk/5q2/3p3p/3Pp2P/2P3P1/5P2/rQR3K1 b - - 6 39 c0 9836741797848742797; c1 15235712741014048904; c2 10248193; c3 0",
			"8/5ppk/5q2/3p3p/3Pp2P/2P3P1/r4P2/rQR3K1 w - - 7 40 c0 9838168399220317068; c1 15235714305392414984; c2 10514433; c3 0",
			"8/5ppk/5q2/3p3p/3PpP1P/2P3P1/r7/rQR3K1 b - f3 0 40 c0 9838168399220317069; c1 15235702845409341704; c2 671176705; c3 0",
			"8/5ppk/6q1/3p3p/3PpP1P/2P3P1/r7/rQR3K1 w - - 1 41 c0 9838223374801705868; c1 15235702845409341704; c2 10752001; c3 0",
			"8/5ppk/6q1/3p3p/3PpP1P/2P3P1/r7/rQR2K2 b - - 2 41 c0 9838223374801705867; c1 15235702845409341704; c2 10756097; c3 0",
			"8/5ppk/8/3p3p/3PpPqP/2P3P1/r7/rQR2K2 w - - 3 42 c0 9838254161127283594; c1 15235702845409739536; c2 11022337; c3 0",
			"8/5ppk/8/3p3p/3PpPqP/2P3P1/Q7/r1R2K2 b - - 0 42 c0 9838254161127283595; c1 2121205337344066320; c2 688128; c3 0",
			"8/5ppk/8/3p3p/3PpP1P/2P2qP1/Q7/r1R2K2 w - - 1 43 c0 9838254161127283594; c1 2121205350702023440; c2 704768; c3 0",
			"8/5ppk/8/3p3p/3PpP1P/2P2qP1/Q7/r1R1K3 b - - 2 43 c0 9838254161127283593; c1 2121205350702023440; c2 705024; c3 0",
			"8/5ppk/8/3p3p/3PpP1P/2P2qP1/Q7/2r1K3 w - - 0 44 c0 9838254161127283592; c1 414341091928605456; c2 45056; c3 0",
			"8/5ppk/8/3p3p/3PpP1P/2P2qP1/Q2K4/2r5 b - - 1 44 c0 9838254161127283607; c1 207175509069562640; c2 45072; c3 0",
			"rn1q1rk1/pbppnpbp/1p2p1p1/4P1B1/2PP4/3B1N2/PP3PPP/RN1Q1RK1 b - - 4 8 c0 3714461534258134797; c1 37176757036656854; c2 16466972800303826472; c3 132096",
			"rn1q1rk1/pb1pnpbp/1p2p1p1/2p1P1B1/2PP4/3B1N2/PP3PPP/RN1Q1RK1 w - - 0 9 c0 7149652518779158284; c1 37176774203875981; c2 16466972800303826472; c3 147456",
			"rn1q1rk1/pb1pnpbp/1p2p1p1/2p1P3/2PP4/3B1N2/PP3PPP/RNBQ1RK1 b - - 1 9 c0 7149652518779158285; c1 9225694292458152589; c2 16470903022365577314; c3 147712",
			"rn2qrk1/pb1pnpbp/1p2p1p1/2p1P3/2PP4/3B1N2/PP3PPP/RNBQ1RK1 w - - 2 10 c0 7149652519198588684; c1 9225694292458152589; c2 16470903022365577314; c3 164352",
			"rn2qrk1/pb1pnpbp/1p2p1p1/2P1P3/2P5/3B1N2/PP3PPP/RNBQ1RK1 b - - 0 10 c0 7149652519198588685; c1 2882374200126939789; c2 1029431438897848582; c3 10240",
			"rn2qrk1/pb1pnpbp/4p1p1/2p1P3/2P5/3B1N2/PP3PPP/RNBQ1RK1 w - - 0 11 c0 7149652519198588684; c1 7097677416222757005; c2 64339464931115536; c3 704",
			"rn2qrk1/pb1pnpbp/4p1p1/2p1P3/2P5/1Q1B1N2/PP3PPP/RNB2RK1 b - - 1 11 c0 7149652519198588685; c1 2963372958296641677; c2 63409318826877190; c3 705",
			"rn2qrk1/p2pnpbp/4p1p1/2p1P3/2P5/1Q1B1b2/PP3PPP/RNB2RK1 w - - 0 12 c0 15434831075281403660; c1 11714425855962009864; c2 3963082426679826; c3 48",
			"rn2qrk1/p2pnpbp/4p1p1/2p1P3/2P5/1Q1B1P2/PP3P1P/RNB2RK1 b - - 0 12 c0 15434831075281403661; c1 2491053819107234056; c2 247692651528720; c3 3",
			"r3qrk1/p2pnpbp/2n1p1p1/2p1P3/2P5/1Q1B1P2/PP3P1P/RNB2RK1 w - - 1 13 c0 10188048979059736332; c1 2491053819107234124; c2 4683991305116844560; c3 3",
			"r3qrk1/p2pnpbp/2n1p1p1/2p1P3/2P2P2/1Q1B4/PP3P1P/RNB2RK1 b - - 0 13 c0 10188048979059736333; c1 2963404014399803724; c2 4611933711078916624; c3 3",
			"3rqrk1/p2pnpbp/2n1p1p1/2p1P3/2P2P2/1Q1B4/PP3P1P/RNB2RK1 w - - 1 14 c0 10188048979061055244; c1 2963404014399803724; c2 9295677323544232464; c3 3",
			"3rqrk1/p2pnpbp/2n1p1p1/2p1P3/2P2P2/1QNB4/PP3P1P/R1B2RK1 b - - 2 14 c0 10188048979061055245; c1 10169163418192597324; c2 9367734920125882626; c3 3",
			"3rqrk1/p3npbp/2npp1p1/2p1P3/2P2P2/1QNB4/PP3P1P/R1B2RK1 w - - 0 15 c0 14471810602306576140; c1 10169163418192597388; c2 13835305750477414658; c3 3",
			"3rqrk1/p3npbp/2npp1p1/2p1P3/2P1NP2/1Q1B4/PP3P1P/R1B2RK1 b - - 1 15 c0 14471810602306576141; c1 10521024731266961804; c2 13907363344515342594; c3 3",
			"3rqrk1/p3npbp/3pp1p1/2p1P3/2PnNP2/1Q1B4/PP3P1P/R1B2RK1 w - - 2 16 c0 9860124583879188236; c1 10521034918610617624; c2 144362883271106818; c3 4",
			"3rqrk1/p3npbp/3pp1p1/2p1P3/2PnNP2/3B4/PP3P1P/R1BQ1RK1 b - - 3 16 c0 9860124583879188237; c1 2882929950590256408; c2 216424108307644944; c3 4",
			"3rqrk1/p4pbp/3pp1p1/2p1Pn2/2PnNP2/3B4/PP3P1P/R1BQ1RK1 w - - 4 17 c0 9839628587625815820; c1 2882929951207867473; c2 4900167720772960784; c3 4",
			"3rqrk1/p4pbp/3pp1p1/2p1Pn2/2PnNP2/3B4/PP1B1P1P/R2Q1RK1 b - - 5 17 c0 9839628587625815821; c1 2882929951207867473; c2 4972225257225224720; c3 4",
			"3r1rk1/p4pbp/2qpp1p1/2p1Pn2/2PnNP2/3B4/PP1B1P1P/R2Q1RK1 w - - 6 18 c0 10252679989297618700; c1 2882929951207867473; c2 9655968869690540560; c3 4",
			"3r1rk1/p4pbp/2qpp1p1/2p1Pn2/2PnNP2/3B4/PP1B1P1P/R2QR1K1 b - - 7 18 c0 10252679989297618701; c1 2882929951207867473; c2 9727903318426157584; c3 4",
			"3r1rk1/p4pbp/2qpp1p1/2p1P3/2PnNP1n/3B4/PP1B1P1P/R2QR1K1 w - - 8 19 c0 10252679989297618700; c1 2925127094794208337; c2 14411646930891473424; c3 4",
			"3r1rk1/p4pbp/2qpp1p1/2p1P3/2PnNP1n/3BR3/PP1B1P1P/R2Q2K1 b - - 9 19 c0 10252679989297618701; c1 2925127094794208337; c2 14483656049680720135; c3 4",
			"3r1rk1/p4pbp/2q1p1p1/2p1p3/2PnNP1n/3BR3/PP1B1P1P/R2Q2K1 w - - 0 20 c0 1605768704746266380; c1 8253270975673615429; c2 5764612501037810192; c3 0",
			"3r1rk1/p4pbp/2q1p1p1/2p1P3/2PnN2n/3BR3/PP1B1P1P/R2Q2K1 b - - 0 20 c0 1605768704746266381; c1 515829530129155141; c2 360288281314863137; c3 0",
			"3r1rk1/p4pbp/2q1p1p1/2p1Pn2/2P1N2n/3BR3/PP1B1P1P/R2Q2K1 w - - 1 21 c0 1605768704746266380; c1 515829490373444677; c2 378584154801055777; c3 0",
			"3r1rk1/p4pbp/2q1p1p1/2p1Pn2/2P1N2n/2BBR3/PP3P1P/R2Q2K1 b - - 2 21 c0 1605768704746266381; c1 8256391349916484677; c2 378865629777756688; c3 0",
			"3r1rk1/p4pbp/2q1p1p1/2p1P3/2P1N2n/2BBn3/PP3P1P/R2Q2K1 w - - 0 22 c0 1605768704746266380; c1 1380715587822502981; c2 24769817395863585; c3 0",
			"3r1rk1/p4pbp/2q1p1p1/2p1P3/2P1N2n/2BBP3/PP5P/R2Q2K1 b - - 0 22 c0 1605768704746266381; c1 83678895139800133; c2 1548113587240993; c3 0",
			"3r1rk1/p2q1pbp/4p1p1/2p1P3/2P1N2n/2BBP3/PP5P/R2Q2K1 w - - 1 23 c0 1192718092117417740; c1 83678895139800133; c2 1619581843046433; c3 0",
			"3r1rk1/p2q1pbp/4p1p1/2p1P3/2P1N2n/2B1P3/PPB4P/R2Q2K1 b - - 2 23 c0 1192718092117417741; c1 1157787406267663429; c2 1620681354674498; c3 0",
			"3r1rk1/p4pbp/4p1p1/2p1P3/2P1N2n/2B1P3/PPB4P/R2q2K1 w - - 0 24 c0 5839152354474532620; c1 2378204722105422916; c2 105553326442516; c3 0",
			"3r1rk1/p4pbp/4p1p1/2p1P3/2P1N2n/2B1P3/PPB4P/3R2K1 b - - 0 24 c0 5839152354474532621; c1 2378204722105422916; c2 6597073438740; c3 0",
			"5rk1/p4pbp/4p1p1/2p1P3/2P1N2n/2B1P3/PPB4P/3r2K1 w - - 0 25 c0 4976633040581959436; c1 4760323813558976836; c2 429497483393; c3 0",
			"5rk1/p4pbp/4p1p1/2p1P3/2P1N2n/2B1P3/PP5P/3B2K1 b - - 0 25 c0 4976633040581959437; c1 148637795131588932; c2 26843555848; c3 0",
			"5rk1/p4pbp/4p1p1/2p1Pn2/2P1N3/2B1P3/PP5P/3B2K1 w - - 1 26 c0 4976633040581959436; c1 148637755088462148; c2 27934074888; c3 0",
			"5rk1/p4pbp/4p1p1/2p1Pn2/2P1N3/2B1P3/PP3K1P/3B4 b - - 2 26 c0 4976633040581959451; c1 148637755088462148; c2 27950846980; c3 0",
			"5rk1/p4p1p/4p1pb/2p1Pn2/2P1N3/2B1P3/PP3K1P/3B4 w - - 3 27 c0 12416715139806142234; c1 148637755088462148; c2 29041366020; c3 0",
			"5rk1/p4p1p/4p1pb/2p1Pn2/2P1N3/4P3/PP1B1K1P/3B4 b - - 4 27 c0 12416715139806142235; c1 2891593536069853508; c2 29058143236; c3 0",
			"1r4k1/p4p1p/4p1pb/2p1Pn2/2P1N3/4P3/PP1B1K1P/3B4 w - - 5 28 c0 12416715139800489754; c1 2891593536069853508; c2 30148662276; c3 0",
			"1r4k1/p4p1p/4p1pb/2p1Pn2/2P1N3/1P2P3/P2B1K1P/3B4 b - - 0 28 c0 12416715139800489755; c1 2886825091579142468; c2 30064776196; c3 0",
			"1r4k1/5p1p/p3p1pb/2p1Pn2/2P1N3/1P2P3/P2B1K1P/3B4 w - - 0 29 c0 12416724932577582874; c1 2886825091579142468; c2 31138518020; c3 0",
			"1r4k1/5p1p/p3p1pb/2p1Pn2/2P1N3/1P2P3/P2BBK1P/8 b - - 1 29 c0 12416724932577582875; c1 2886825091579142468; c2 31155290181; c3 0",
			"3r2k1/5p1p/p3p1pb/2p1Pn2/2P1N3/1P2P3/P2BBK1P/8 w - - 2 30 c0 12416724932578713370; c1 2886825091579142468; c2 32245809221; c3 0",
			"3r2k1/5p1p/p3p1pb/2p1Pn2/2P1N3/1P2PB2/P2B1K1P/8 b - - 3 30 c0 12416724932578713371; c1 9298262111094464836; c2 32262586434; c3 0",
			"3r1bk1/5p1p/p3p1p1/2p1Pn2/2P1N3/1P2PB2/P2B1K1P/8 w - - 4 31 c0 4976786147374472986; c1 9298262111094464836; c2 33353105474; c3 0",
			"3r1bk1/5p1p/p3p1p1/B1p1Pn2/2P1N3/1P2PB2/P4K1P/8 b - - 5 31 c0 4976786147374472987; c1 1198241187835024453; c2 33369882688; c3 0",
			"2r2bk1/5p1p/p3p1p1/B1p1Pn2/2P1N3/1P2PB2/P4K1P/8 w - - 6 32 c0 4976786147373719322; c1 1198241187835024453; c2 34460401728; c3 0",
			"2r2bk1/5p1p/p3p1p1/B1p1Pn2/2P5/1PN1PB2/P4K1P/8 b - - 7 32 c0 4976786147373719323; c1 1198252307505353797; c2 34477178944; c3 0",
			"2r3k1/5p1p/p3p1pb/B1p1Pn2/2P5/1PN1PB2/P4K1P/8 w - - 8 33 c0 12416724932577959706; c1 1198252307505353797; c2 35567697984; c3 0",
			"2r3k1/5p1p/p3p1pb/B1p1Pn2/2P5/1P2PB2/P4K1P/3N4 b - - 9 33 c0 12416724932577959707; c1 74890073499636805; c2 35584478212; c3 0",
			"2r3k1/5p1p/p3p1p1/B1p1Pnb1/2P5/1P2PB2/P4K1P/3N4 w - - 10 34 c0 6075656657240301338; c1 74890073502802244; c2 36674997252; c3 0",
			"2r3k1/5p1p/p3p1p1/B1p1Pnb1/2P5/1P2PB2/P3K2P/3N4 b - - 11 34 c0 6075656657240301337; c1 74890073502802244; c2 36691774468; c3 0",
			"2r3k1/5p1p/p3p1pb/B1p1Pn2/2P5/1P2PB2/P3K2P/3N4 w - - 12 35 c0 12416724932577959704; c1 74890073499636805; c2 37782293508; c3 0",
			"2r3k1/1B3p1p/p3p1pb/B1p1Pn2/2P5/1P2P3/P3K2P/3N4 b - - 13 35 c0 14200158184308186905; c1 72341269151564890; c2 37799070724; c3 0",
			"5rk1/1B3p1p/p3p1pb/B1p1Pn2/2P5/1P2P3/P3K2P/3N4 w - - 14 36 c0 14200158184313462552; c1 72341269151564890; c2 38889589764; c3 0",
			"5rk1/1B3p1p/pB2p1pb/2p1Pn2/2P5/1P2P3/P3K2P/3N4 b - - 15 36 c0 5839788426053558041; c1 72341269151564972; c2 38906366980; c3 0",
			"1r4k1/1B3p1p/pB2p1pb/2p1Pn2/2P5/1P2P3/P3K2P/3N4 w - - 16 37 c0 5839788426047905560; c1 72341269151564972; c2 39996886020; c3 0",
			"1r4k1/5p1p/BB2p1pb/2p1Pn2/2P5/1P2P3/P3K2P/3N4 b - - 0 37 c0 14200031637760499481; c1 4616207347749360714; c2 2483028160; c3 0",
			"6k1/5p1p/Br2p1pb/2p1Pn2/2P5/1P2P3/P3K2P/3N4 w - - 0 38 c0 12416756605847084824; c1 288512959234335044; c2 159383564; c3 0",
			"6k1/5p1p/1r2p1pb/1Bp1Pn2/2P5/1P2P3/P3K2P/3N4 b - - 1 38 c0 12305262347219377945; c1 288512959234335044; c2 159449100; c3 0",
			"1r4k1/5p1p/4p1pb/1Bp1Pn2/2P5/1P2P3/P3K2P/3N4 w - - 2 39 c0 12305259742288535320; c1 288512959234335044; c2 163708940; c3 0",
			"1r4k1/5p1p/2B1p1pb/2p1Pn2/2P5/1P2P3/P3K2P/3N4 b - - 3 39 c0 12416728231112466201; c1 288512959234335044; c2 163774476; c3 0",
			"1r4k1/5pbp/2B1p1p1/2p1Pn2/2P5/1P2P3/P3K2P/3N4 w - - 4 40 c0 4976839147718565656; c1 288512959234335044; c2 168034316; c3 0",
			"1r4k1/5pbp/2B1p1p1/2p1Pn2/2P5/1P2P3/P3KN1P/8 b - - 5 40 c0 4976839147718565657; c1 4828141383623795012; c2 168099840; c3 0",
			"1r4k1/5p1p/2B1p1p1/2p1bn2/2P5/1P2P3/P3KN1P/8 w - - 0 41 c0 4922738451167960856; c1 301758836476487508; c2 10747904; c3 0",
			"1r4k1/5p1p/2B1p1p1/2p1bn2/2P3N1/1P2P3/P3K2P/8 b - - 1 41 c0 4922738451167960857; c1 288512959431583572; c2 10752000; c3 0",
			"1r4k1/2b2p1p/2B1p1p1/2p2n2/2P3N1/1P2P3/P3K2P/8 w - - 2 42 c0 4976838925252681496; c1 288512959431583300; c2 11018240; c3 0",
			"1r4k1/2b2p1p/2B1p1p1/2p2n2/2P5/1P2P3/P3KN1P/8 b - - 3 42 c0 4976838925252681497; c1 301758836476487236; c2 11022336; c3 0",
			"1r3k2/2b2p1p/2B1p1p1/2p2n2/2P5/1P2P3/P3KN1P/8 w - - 4 43 c0 4976838925252681368; c1 301758836476487236; c2 11288576; c3 0",
			"1r3k2/2b2p1p/2B1p1p1/2p2n2/2P1N3/1P2P3/P3K2P/8 b - - 5 43 c0 4976838925252681369; c1 288512959280588356; c2 11292672; c3 0",
			"1r3k2/5p1p/2Bbp1p1/2p2n2/2P1N3/1P2P3/P3K2P/8 w - - 6 44 c0 4979455658975157912; c1 288512959280588356; c2 11558912; c3 0",
			"1r3k2/5p1p/2Bbp1p1/2p2n2/2P5/1P2P3/P3KN1P/8 b - - 7 44 c0 4979455658975157913; c1 301758836476487236; c2 11563008; c3 0",
			"5k2/5p1p/1rBbp1p1/2p2n2/2P5/1P2P3/P3KN1P/8 w - - 8 45 c0 4979458263906000536; c1 301758836476487236; c2 11829248; c3 0",
			"5k2/5p1p/1r1bp1p1/1Bp2n2/2P5/1P2P3/P3KN1P/8 b - - 9 45 c0 11840432811460796057; c1 301758836476487236; c2 11833344; c3 0",
			"8/4kp1p/1r1bp1p1/1Bp2n2/2P5/1P2P3/P3KN1P/8 w - - 10 46 c0 11840432811460794904; c1 301758836476487236; c2 12099584; c3 0",
			"8/4kp1p/1r1bp1p1/1Bp2n2/2P5/1P2P2P/P3KN2/8 b - - 0 46 c0 11840432811460794905; c1 216463122440136260; c2 12058624; c3 0",
			"8/4kp2/1r1bp1p1/1Bp2n1p/2P5/1P2P2P/P3KN2/8 w - - 0 47 c0 5351713067396766232; c1 216463122440161892; c2 12320768; c3 0",
			"8/4kp2/1r1bp1p1/1Bp2n1p/P1P5/1P2P2P/4KN2/8 b - - 0 47 c0 5351713067396766233; c1 216314627707996772; c2 12320768; c3 0",
			"8/1r2kp2/3bp1p1/1Bp2n1p/P1P5/1P2P2P/4KN2/8 w - - 1 48 c0 5351712878481119768; c1 216314627707996772; c2 12587008; c3 0",
			"8/1r2kp2/3bp1p1/PBp2n1p/2P5/1P2P2P/4KN2/8 b - - 0 48 c0 11620723559780850201; c1 216314627708462660; c2 12582912; c3 0",
			"8/2r1kp2/3bp1p1/PBp2n1p/2P5/1P2P2P/4KN2/8 w - - 1 49 c0 11620723559877319192; c1 216314627708462660; c2 12849152; c3 0",
			"8/2r1kp2/P2bp1p1/1Bp2n1p/2P5/1P2P2P/4KN2/8 b - - 0 49 c0 11840429691326437913; c1 216314627708462660; c2 12845056; c3 0",
			"5k2/2r2p2/P2bp1p1/1Bp2n1p/2P5/1P2P2P/4KN2/8 w - - 1 50 c0 11840429691229970072; c1 216314627708462660; c2 13111296; c3 0",
			"5k2/2r2p2/P2bp1p1/1Bp2n1p/2P5/1P2P2P/3K1N2/8 b - - 2 50 c0 11840429691229970071; c1 216314627708462660; c2 13115392; c3 0",
			"5k2/2r2p2/P3p1p1/1Bp1bn1p/2P5/1P2P2P/3K1N2/8 w - - 3 51 c0 5351701402328506006; c1 216314627708462932; c2 13381632; c3 0",
			"5k2/2r2p2/P3p1p1/1Bp1bn1p/2P5/1P1NP2P/3K4/8 b - - 4 51 c0 5351701402328506007; c1 2271049861867348; c2 13385728; c3 0",
			"5k2/2r2p2/P3p1p1/1Bp2n1p/2P5/1P1NP1bP/3K4/8 w - - 5 52 c0 5351701402328506006; c1 3730746569427556; c2 13651968; c3 0",
			"5k2/2r2p2/P3p1p1/1Bp2n1p/1PP5/3NP1bP/3K4/8 b - - 0 52 c0 5351701402328506007; c1 3730746036619876; c2 13631488; c3 0",
			"5k2/2r2p2/P2bp1p1/1Bp2n1p/1PP5/3NP2P/3K4/8 w - - 1 53 c0 11840429691229970070; c1 2271041336944196; c2 13897728; c3 0",
			"5k2/2r2p2/P2bp1p1/1Bp2n1p/1PP1P3/3N3P/3K4/8 b - - 0 53 c0 11840429691229970071; c1 2278192457492036; c2 13893632; c3 0",
			"5k2/2r2p2/P2bp1p1/1Bp4p/1PPnP3/3N3P/3K4/8 w - - 1 54 c0 11840429691229970070; c1 2278194942216260; c2 14159872; c3 0",
			"5k2/2r2p2/P2bp1p1/1BP4p/2PnP3/3N3P/3K4/8 b - - 0 54 c0 11840429691229970071; c1 142387183912964; c2 884736; c3 0",
			"5k2/2r2p2/P3p1p1/1Bb4p/2PnP3/3N3P/3K4/8 w - - 0 55 c0 5351701402328506006; c1 8899198994565; c2 56320; c3 0",
			"5k2/2r2p2/P3p1p1/2b4p/B1PnP3/3N3P/3K4/8 b - - 1 55 c0 6072277342707785367; c1 8899198993800; c2 56336; c3 0",
			"5k2/2r5/P3ppp1/2b4p/B1PnP3/3N3P/3K4/8 w - - 0 56 c0 6072595028016897686; c1 8899198993800; c2 57344; c3 0",
			"5k2/2r5/P3ppp1/2b4p/B1PnP3/3NK2P/8/8 b - - 1 56 c0 6072595028016897705; c1 4501152482696; c2 57360; c3 0",
			"5k2/2r5/Pb2ppp1/7p/B1PnP3/3NK2P/8/8 w - - 2 57 c0 316116881641578152; c1 4501152482696; c2 58400; c3 0",
			"5k2/2r5/Pb2ppp1/2P4p/B2nP3/3NK2P/8/8 b - - 0 57 c0 4927802900068966057; c1 4501152487552; c2 58368; c3 0",
			"5k2/8/Pb2ppp1/2r4p/B2nP3/3NK2P/8/8 w - - 0 58 c0 6649055956585946792; c1 281322030472; c2 3712; c3 0",
			"5k2/8/Pb2ppp1/2N4p/B2nP3/4K2P/8/8 b - - 0 58 c0 884448433551711913; c1 17181541768; c2 232; c3 0",
			"5k2/8/P3ppp1/2b4p/B2nP3/4K2P/8/8 w - - 0 59 c0 9602909226099809960; c1 13835058056356010072; c2 14; c3 0",
			"5k2/8/P3ppp1/2b4p/B2nP3/3K3P/8/8 b - - 1 59 c0 9602909226099809959; c1 13907115650393938008; c2 14; c3 0",
			"8/4k3/P3ppp1/2b4p/B2nP3/3K3P/8/8 w - - 2 60 c0 9602909226099808806; c1 144115189149702232; c2 15; c3 0",
			"8/4k3/P3ppp1/2b1P2p/B2n4/3K3P/8/8 b - - 0 60 c0 1532458693851879975; c1 1074365832; c2 15; c3 0",
			"8/4k3/P3p1p1/2b1p2p/B2n4/3K3P/8/8 w - - 0 61 c0 10472070968833481254; c1 17582052945321564248; c2 0; c3 0",
			"8/4k3/P3p1p1/2b1p2p/3n4/3K3P/8/3B4 b - - 1 61 c0 10472070968833481255; c1 17586557919275518344; c2 0; c3 0",
			"8/8/P2kp1p1/2b1p2p/3n4/3K3P/8/3B4 w - - 2 62 c0 10472070969101915558; c1 17879291895054600584; c2 0; c3 0",
			"8/8/P2kp1p1/2b1p2p/3n4/4K2P/8/3B4 b - - 3 62 c0 10472070969101915561; c1 17883795494681971080; c2 0; c3 0",
			"8/8/P2kp1p1/1nb1p2p/8/4K2P/8/3B4 w - - 4 63 c0 1541995857979643304; c1 18176529470461051017; c2 0; c3 0",
			"8/8/P2kp1p1/1nb1p2p/8/3K3P/8/3B4 b - - 5 63 c0 1541995857979643303; c1 18181033070088421513; c2 0; c3 0",
			"8/2n5/P2kp1p1/2b1p2p/8/3K3P/8/3B4 w - - 6 64 c0 1532438842403984806; c1 27022972157952137; c2 1; c3 0",
			"8/2n5/P2kp1p1/2b1p2p/8/3K1B1P/8/8 b - - 7 64 c0 1532438842403984807; c1 31525197463945353; c2 1; c3 0",
			"8/8/n2kp1p1/2b1p2p/8/3K1B1P/8/8 w - - 0 65 c0 10472070978765591974; c1 1170935903120850952; c2 0; c3 0",
			"8/8/n2kp1p1/2b1p2p/8/5B1P/3K4/8 b - - 1 65 c0 10472070978765591959; c1 1171217378102083592; c2 0; c3 0",
			"8/8/3kp1p1/2b1p2p/1n6/5B1P/3K4/8 w - - 2 66 c0 9877876472390096278; c1 1189513251588276262; c2 0; c3 0",
			"8/8/3kp1p1/2b1p2p/1n2B3/7P/3K4/8 b - - 3 66 c0 9877876472390096279; c1 1189794726564332838; c2 0; c3 0",
			"8/8/3kp3/2b1p1pp/1n2B3/7P/3K4/8 w - - 0 67 c0 10110955070414656918; c1 1206964700143682854; c2 0; c3 0",
			"8/8/3kp3/2b1p1pp/1n2B3/7P/8/4K3 b - - 1 67 c0 10110955070414656905; c1 1207246175120393510; c2 0; c3 0",
			"8/8/2nkp3/2b1p1pp/4B3/7P/8/4K3 w - - 2 68 c0 14201328577760204168; c1 1225542048606586120; c2 0; c3 0",
			"8/8/2nkp3/2b1p1pp/4B3/7P/4K3/8 b - - 3 68 c0 14201328577760204185; c1 1225823523583296776; c2 0; c3 0",
			"8/4n3/3kp3/2b1p1pp/4B3/7P/4K3/8 w - - 4 69 c0 14201328537595549080; c1 1244119397069489416; c2 0; c3 0",
			"8/4n3/3kp3/2b1p1pp/4B3/5K1P/8/8 b - - 5 69 c0 14201328537595549099; c1 1244400872042005768; c2 0; c3 0",
			"8/4n3/3kp3/4p1pp/1b2B3/5K1P/8/8 w - - 6 70 c0 10110863807292118442; c1 1262696745528198442; c2 0; c3 0",
			"8/4n2B/3kp3/4p1pp/1b6/5K1P/8/8 b - - 7 70 c0 14199868338909222315; c1 1262978220504908456; c2 0; c3 0",
			"8/7B/2nkp3/4p1pp/1b6/5K1P/8/8 w - - 8 71 c0 14199868970974057898; c1 1281274093991101096; c2 0; c3 0",
			"8/8/2nkp3/4p1pp/1b2B3/5K1P/8/8 b - - 9 71 c0 10110863847456773547; c1 1281555568967812394; c2 0; c3 0",
			"r1bqkb1r/ppp1n3/2n2pp1/3pp2p/4P3/2PP3P/PP1NNPP1/R1BQKB1R b KQkq - 1 8 c0 1388852213139045897; c1 594493331820798540; c2 14215971781865906440; c3 131449",
			"r1bqkb1r/ppp1n3/2n3p1/3ppp1p/4P3/2PP3P/PP1NNPP1/R1BQKB1R w KQkq - 0 9 c0 1388852213139045896; c1 594493336535450700; c2 14215971781865906440; c3 147577",
			"r1bqkb1r/ppp1n3/2n3p1/3Ppp1p/8/2PP3P/PP1NNPP1/R1BQKB1R b KQkq - 0 9 c0 1388852213139045897; c1 9260527327308694604; c2 11264791777828241936; c3 9223",
			"r1bqkb1r/ppp1n3/2n3p1/3P1p1p/4p3/2PP3P/PP1NNPP1/R1BQKB1R w KQkq - 0 10 c0 1388852213139045896; c1 9260545462556443724; c2 11264791777828241936; c3 10247",
			"r1bqkb1r/ppp1n3/2n3p1/3P1p1p/4P3/2P4P/PP1NNPP1/R1BQKB1R b KQkq - 0 10 c0 1388852213139045897; c1 577587788800083020; c2 8774500018362193953; c3 640",
			"r1bqkb1r/ppp1n3/6p1/3Pnp1p/4P3/2P4P/PP1NNPP1/R1BQKB1R w KQkq - 1 11 c0 1388852213139045896; c1 577587788819498048; c2 8774500018362193953; c3 705",
			"r1bqkb1r/ppp1n3/6p1/3Pnp1p/3NP3/2P4P/PP1N1PP1/R1BQKB1R b KQkq - 2 11 c0 1388852213139045897; c1 9241405713915348032; c2 8774500018362090000; c3 706",
			"r1bq1b1r/ppp1nk2/6p1/3Pnp1p/3NP3/2P4P/PP1N1PP1/R1BQKB1R w KQ - 3 12 c0 2777704425847454344; c1 9241405713915348032; c2 1856970990721008144; c3 771",
			"r1bq1b1r/ppp1nk2/6p1/3Pnp1p/3NPP2/2P4P/PP1N2P1/R1BQKB1R b KQ - 0 12 c0 2777704425847454345; c1 288812604656223296; c2 1856970990720393480; c3 768",
			"r1bq1b1r/ppp2k2/6p1/3nnp1p/3NPP2/2P4P/PP1N2P1/R1BQKB1R w KQ - 0 13 c0 39515852406192776; c1 9241422824645826628; c2 116060686920024592; c3 52",
			"r1bq1b1r/ppp2k2/6p1/3Pnp1p/3N1P2/2P4P/PP1N2P1/R1BQKB1R b KQ - 0 13 c0 39515852406192777; c1 577588956631337028; c2 4618939811359889441; c3 3",
			"r1b2b1r/ppp2k2/6p1/3Pnp1p/3N1P1q/2P4P/PP1N2P1/R1BQKB1R w KQ - 1 14 c0 4614155759187065480; c1 577806698562138244; c2 9302683423825205281; c3 3",
			"r1b2b1r/ppp2k2/6p1/3Pnp1p/3N1P1q/2P4P/PP1NK1P1/R1BQ1B1R b - - 2 14 c0 4614155759187065497; c1 577806698562138244; c2 9367985461060048929; c3 3",
			"r1b2b1r/ppp2k2/6p1/3Pnp1p/3N1q2/2P4P/PP1NK1P1/R1BQ1B1R w - - 0 15 c0 4614155759187065496; c1 1189024112931188868; c2 4323486782033764738; c3 0",
			"r1b2b1r/ppp2k2/6p1/3Pnp1p/3N1q2/2P3PP/PP1NK3/R1BQ1B1R b - - 0 15 c0 4614155759187065497; c1 594548962118283396; c2 4323486782033762337; c3 0",
			"r1b2b1r/ppp2k2/6p1/3Pnp1p/3N4/2P3qP/PP1NK3/R1BQ1B1R w - - 0 16 c0 4614155759187065496; c1 1217102203930751108; c2 288232322386592130; c3 0",
			"r1b2b1r/ppp2k2/6p1/3Pnp1p/3N4/2P2NqP/PP2K3/R1BQ1B1R b - - 1 16 c0 4614155759187065497; c1 1028513969534611588; c2 288513797363302433; c3 0",
			"r1b4r/ppp2k2/3b2p1/3Pnp1p/3N4/2P2NqP/PP2K3/R1BQ1B1R w - - 2 17 c0 5368445114545535640; c1 1028513969534611588; c2 306809670849495073; c3 0",
			"r1b4r/ppp2k2/3b2p1/3PnpNp/3N4/2P3qP/PP2K3/R1BQ1B1R b - - 3 17 c0 5368445114545535641; c1 1026891189160456324; c2 307091145826205729; c3 0",
			"r1b3kr/ppp5/3b2p1/3PnpNp/3N4/2P3qP/PP2K3/R1BQ1B1R w - - 4 18 c0 5368367935191187224; c1 1026891189160456324; c2 325387019312398369; c3 0",
			"r1b3kr/ppp5/3b2p1/3PnpNp/3N4/2P3qP/PP2K3/R1B1QB1R b - - 5 18 c0 5368367935191187225; c1 1026891189160456324; c2 325668494893088801; c3 0",
			"r1b3kr/ppp5/3b2p1/3PnpN1/3N3p/2P3qP/PP2K3/R1B1QB1R w - - 0 19 c0 5368367935191187224; c1 1026900436090826884; c2 342275518519017505; c3 0",
			"r1b3kr/ppp5/3b2p1/3PnpN1/3N3p/2P3QP/PP2K3/R1B2B1R b - - 0 19 c0 5368367935191187225; c1 738670059939115140; c2 21392219841986593; c3 0",
			"r1b3kr/ppp5/3b2p1/3PnpN1/3N4/2P3pP/PP2K3/R1B2B1R w - - 0 20 c0 5368367935191187224; c1 1208095004545986692; c2 1407382484301826; c3 0",
			"r1b3kr/ppp5/3b2p1/3PnpN1/3N4/2P3pP/PP2K1B1/R1B4R b - - 1 20 c0 5368367935191187225; c1 1208095004545986692; c2 1408481922091522; c3 0",
			"r1b3kr/pp6/2pb2p1/3PnpN1/3N4/2P3pP/PP2K1B1/R1B4R w - - 0 21 c0 5387433466816823064; c1 1208095004545986692; c2 1477751154641410; c3 0",
			"r1b3kr/pp6/2pb2p1/3PnpN1/3N1B2/2P3pP/PP2K1B1/R6R b - - 1 21 c0 5387433466816823065; c1 882776589360902276; c2 1478850659262497; c3 0",
			"r1b3k1/pp6/2pb2p1/3PnpNr/3N1B2/2P3pP/PP2K1B1/R6R w - - 2 22 c0 4948400609921007384; c1 882776589539881736; c2 1550318915067937; c3 0",
			"r1b3k1/pp6/2pb2p1/3PnpNr/3N1B1P/2P3p1/PP2K1B1/R6R b - - 0 22 c0 4948400609921007385; c1 4901051784410051336; c2 1548119891812385; c3 0",
			"r1b3k1/1p6/p1pb2p1/3PnpNr/3N1B1P/2P3p1/PP2K1B1/R6R w - - 0 23 c0 4948419160153194264; c1 4901051784410051336; c2 1618488635990049; c3 0",
			"r1b3k1/1p6/p1Pb2p1/4npNr/3N1B1P/2P3p1/PP2K1B1/R6R b - - 0 23 c0 4947293260246351641; c1 1459237241132475184; c2 101155539749378; c3 0",
			"r1b3k1/8/p1pb2p1/4npNr/3N1B1P/2P3p1/PP2K1B1/R6R w - - 0 24 c0 309276196949655320; c1 2397045336784473651; c2 6597099141280; c3 0",
			"r1b3k1/8/p1Nb2p1/4npNr/5B1P/2P3p1/PP2K1B1/R6R b - - 0 24 c0 309214624298499865; c1 149815333548166707; c2 412318696330; c3 0",
			"r1b3k1/8/p1nb2p1/5pNr/5B1P/2P3p1/PP2K1B1/R6R w - - 0 25 c0 309284993042677528; c1 11538578504415230178; c2 26843660344; c3 0",
			"r1b3k1/8/p1nB2p1/5pNr/7P/2P3p1/PP2K1B1/R6R b - - 0 25 c0 307033193228992281; c1 9944533193380116706; c2 1677728771; c3 0",
			"r5k1/3b4/p1nB2p1/5pNr/7P/2P3p1/PP2K1B1/R6R w - - 1 26 c0 307033196036554520; c1 9944533193380116706; c2 1745886211; c3 0",
			"r5k1/3b4/p1nB2p1/5pNr/7P/2P2Kp1/PP4B1/R6R b - - 2 26 c0 307033196036554539; c1 9944240585848174818; c2 1746934787; c3 0",
			"r5kr/3b4/p1nB2p1/5pN1/7P/2P2Kp1/PP4B1/R6R w - - 3 27 c0 4912531136774864682; c1 9944240585848131104; c2 1815092227; c3 0",
			"r5kr/3b4/p1nB2p1/5pN1/7P/2P2Kp1/PP4B1/3R3R b - - 4 27 c0 4912531136774864683; c1 720868548993355296; c2 1816140828; c3 0",
			"r5kr/3b4/2nB2p1/p4pN1/7P/2P2Kp1/PP4B1/3R3R w - - 0 28 c0 1459953574883819306; c1 720868548993355297; c2 1879055388; c3 0",
			"r5kr/3b4/2n3p1/p4pN1/5B1P/2P2Kp1/PP4B1/3R3R b - - 1 28 c0 1244203005235101483; c1 720868548994007266; c2 1880103964; c3 0",
			"6kr/r2b4/2n3p1/p4pN1/5B1P/2P2Kp1/PP4B1/3R3R w - - 2 29 c0 1244203005439909674; c1 720868548994007266; c2 1948261404; c3 0",
			"6kr/r2b4/2n3p1/p4pN1/5B1P/2P3K1/PP4B1/3R3R b - - 0 29 c0 1244203005439909677; c1 13880112194580512994; c2 121635265; c3 0",
			"7r/r2b2k1/2n3p1/p4pN1/5B1P/2P3K1/PP4B1/3R3R w - - 1 30 c0 1244203050934999852; c1 13880112194580512994; c2 125895105; c3 0",
			"7r/r2b2k1/2B3p1/p4pN1/5B1P/2P3K1/PP6/3R3R b - - 0 30 c0 1244141478283844397; c1 2017630776086626530; c2 7864348; c3 0",
			"7r/r5k1/2b3p1/p4pN1/5B1P/2P3K1/PP6/3R3R w - - 0 31 c0 2383606244761934636; c1 13961159978787577870; c2 507905; c3 0",
			"7r/r5k1/2b3p1/p3BpN1/7P/2P3K1/PP6/3R3R b - - 1 31 c0 5842370758582475565; c1 13961159978787537122; c2 508161; c3 0",
			"7r/r7/2b3pk/p3BpN1/7P/2P3K1/PP6/3R3R w - - 2 32 c0 5843572799669540780; c1 13961159978787537122; c2 524801; c3 0",
			"7B/r7/2b3pk/p4pN1/7P/2P3K1/PP6/3R3R b - - 0 32 c0 2384808285830125485; c1 2025494003281068046; c2 32768; c3 0",
			"7B/1r6/2b3pk/p4pN1/7P/2P3K1/PP6/3R3R w - - 1 33 c0 2384808286601877420; c1 2025494003281068046; c2 33808; c3 0",
			"7B/1r6/2b3pk/p4pN1/7P/1PP3K1/P7/3R3R b - - 0 33 c0 2384808286601877421; c1 2025493966838857742; c2 33792; c3 0",
			"7B/3r4/2b3pk/p4pN1/7P/1PP3K1/P7/3R3R w - - 1 34 c0 2384808291232389036; c1 2025493966838857742; c2 34832; c3 0",
			"8/3r4/2b2Bpk/p4pN1/7P/1PP3K1/P7/3R3R b - - 2 34 c0 2384819452353976237; c1 2025493966838857742; c2 34848; c3 0",
			"8/3r4/2b2Bpk/5pN1/p6P/1PP3K1/P7/3R3R w - - 0 35 c0 16287431502046697388; c1 2025493966838858000; c2 35840; c3 0",
			"8/3r4/2b2Bpk/5pN1/pP5P/2P3K1/P7/3R3R b - - 0 35 c0 16287431502046697389; c1 2025493966837252880; c2 35840; c3 0",
			"8/8/2b2Bpk/5pN1/pP5P/2Pr2K1/P7/3R3R w - - 1 36 c0 1017964468853807020; c1 2025493969860853809; c2 36880; c3 0",
			"8/8/2b2Bpk/5pN1/pP3K1P/2Pr4/P7/3R3R b - - 2 36 c0 1017964468853807035; c1 2025493968315236401; c2 36896; c3 0",
			"8/8/2b2Bpk/5pN1/pP3K1P/2P5/P2r4/3R3R w - - 3 37 c0 1017964468853807034; c1 2025500256751337521; c2 37936; c3 0",
			"8/5N2/2b2Bpk/5p2/pP3K1P/2P5/P2r4/3R3R b - - 4 37 c0 2452373446579984315; c1 2025500256751337521; c2 37952; c3 0",
			"8/5N2/2b2Bp1/5p1k/pP3K1P/2P5/P2r4/3R3R w - - 5 38 c0 4902331643869533114; c1 2025500256751337521; c2 38992; c3 0",
			"8/5N2/2b2Bp1/5p1k/pP3K1P/2P5/P2R4/7R b - - 0 38 c0 4902331643869533115; c1 126102715861319729; c2 2432; c3 0",
			"8/5N2/5Bp1/5p1k/pP3K1P/2P5/P2R4/7b w - - 0 39 c0 1459317187440284602; c1 23644018437129219; c2 156; c3 0",
			"3R4/5N2/5Bp1/5p1k/pP3K1P/2P5/P7/7b b - - 1 39 c0 4902330925335385019; c1 1176565404800139313; c2 156; c3 0",
			"3R4/5N2/5B2/5ppk/pP3K1P/2P5/P7/7b w - - 0 40 c0 10106099557273637818; c1 23643900193292337; c2 160; c3 0",
			"3R4/5N2/5B2/5pPk/pP3K2/2P5/P7/7b b - - 0 40 c0 882727520418862011; c1 1477743762079793; c2 10; c3 0",
			"3R4/5N2/5Bk1/5pP1/pP3K2/2P5/P7/7b w - - 1 41 c0 441374756936554298; c1 4685221356227395633; c2 10; c3 0",
			"rn1qkb1r/1p2pppp/p4n2/3P1b2/8/2N5/PP1PBPPP/R1B2KNR b kq - 3 8 c0 14281234078736776715; c1 2378111720766988568; c2 243693645944071432; c3 2",
			"rn1qkb1r/1p3ppp/p4n2/3Ppb2/8/2N5/PP1PBPPP/R1B2KNR w kq e6 0 9 c0 10115685898395778570; c1 2378111721044190737; c2 3234083798518080776; c3 144",
			"rn1qkb1r/1p3ppp/p3Pn2/5b2/8/2N5/PP1PBPPP/R1B2KNR b kq - 0 9 c0 10115685898395778571; c1 9372004019402334481; c2 2595793439354798672; c3 0",
			"rn1qkb1r/1p4pp/p3pn2/5b2/8/2N5/PP1PBPPP/R1B2KNR w kq - 0 10 c0 1784589537713782282; c1 585750251212646161; c2 180251488469156901; c3 0",
			"rn1qkb1r/1p4pp/p3pn2/5b2/8/2N2B2/PP1P1PPP/R1B2KNR b kq - 1 10 c0 1784589537713782283; c1 9372167571096217361; c2 180532963445867552; c3 0",
			"1n1qkb1r/rp4pp/p3pn2/5b2/8/2N2B2/PP1P1PPP/R1B2KNR w k - 2 11 c0 1784590366412168714; c1 9372167571096217361; c2 198758468187882528; c3 0",
			"1n1qkb1r/rp4pp/p3pn2/5b2/8/2N2B2/PP1PNPPP/R1B2K1R b k - 3 11 c0 1784590366412168715; c1 9372167571096217361; c2 199039922538562096; c3 0",
			"1n1qkb1r/rp4pp/p4n2/4pb2/8/2N2B2/PP1PNPPP/R1B2K1R w k - 0 12 c0 1784590366412168714; c1 9372167571097322081; c2 216209896117912112; c3 0",
			"1n1qkb1r/rp4pp/p4n2/4pb2/8/2N2B1P/PP1PNPP1/R1B2K1R b k - 0 12 c0 1784590366412168715; c1 2380329149604627041; c2 216209896117641992; c3 0",
			"1n1qkb1r/rp4pp/p4n2/5b2/4p3/2N2B1P/PP1PNPP1/R1B2K1R w k - 0 13 c0 1784590366412168714; c1 2380329154125628001; c2 234224294627123976; c3 0",
			"1n1qkb1r/rp4pp/p4n2/5b2/4p1B1/2N4P/PP1PNPP1/R1B2K1R b k - 1 13 c0 1784590366412168715; c1 2380165687670342241; c2 234505769603834632; c3 0",
			"1n1qkb1r/rp4pp/p7/5b2/4p1n1/2N4P/PP1PNPP1/R1B2K1R w k - 0 14 c0 1784590366412168714; c1 9372132407366557697; c2 15764918321037872; c3 0",
			"1n1qkb1r/rp4pp/p7/5b2/4p1P1/2N5/PP1PNPP1/R1B2K1R b k - 0 14 c0 1784590366412168715; c1 585750252527722497; c2 985307395064867; c3 0",
			"1n1qkb1r/rp4pp/p7/8/4p1b1/2N5/PP1PNPP1/R1B2K1R w k - 0 15 c0 1784590366412168714; c1 3495373905945690113; c2 65979758702658; c3 0",
			"1n1qkb1r/rp4pp/p7/8/4N1b1/8/PP1PNPP1/R1B2K1R b k - 0 15 c0 1784590366412168715; c1 2524303876437311489; c2 4123734918916; c3 0",
			"1n1q1b1r/rp3kpp/p7/8/4N1b1/8/PP1PNPP1/R1B2K1R w - - 1 16 c0 1784629380452244106; c1 2524303876437311489; c2 4402370922244; c3 0",
			"1n1q1b1r/rp3kpp/p7/8/4N1b1/5P2/PP1PN1P1/R1B2K1R b - - 0 16 c0 1784629380452244107; c1 3495373991830355969; c2 4398075954948; c3 0",
			"1nbq1b1r/rp3kpp/p7/8/4N3/5P2/PP1PN1P1/R1B2K1R w - - 1 17 c0 10107326013532592778; c1 3495373990468255761; c2 4677248829188; c3 0",
			"1nbq1b1r/rp3kpp/p7/8/4N3/5P2/PP1PN1P1/1RB2K1R b - - 2 17 c0 10107326013532592779; c1 3495373990468255761; c2 4681543798276; c3 0",
			"1nbq1bkr/rp4pp/p7/8/4N3/5P2/PP1PN1P1/1RB2K1R w - - 3 18 c0 10106701800165662474; c1 3495373990468255761; c2 4960716672516; c3 0",
			"1nbq1bkr/rp4pp/p7/8/8/5P2/PP1PNNP1/1RB2K1R b - - 4 18 c0 10106701800165662475; c1 7135989902042202129; c2 4965011639812; c3 0",
			"2bq1bkr/rp4pp/p1n5/8/8/5P2/PP1PNNP1/1RB2K1R w - - 5 19 c0 1784590367117188874; c1 7135989902042202189; c2 5244184514052; c3 0",
			"2bq1bkr/rp4pp/p1n5/8/8/2N2P2/PP1P1NP1/1RB2K1R b - - 6 19 c0 1784590367117188875; c1 6954138507459690573; c2 5248479481348; c3 0",
			"2bq1bkr/rp5p/p1n3p1/8/8/2N2P2/PP1P1NP1/1RB2K1R w - - 0 20 c0 15098356765531217674; c1 6954138507459691588; c2 5497587584516; c3 0",
			"2bq1bkr/rp5p/p1n3p1/8/4N3/2N2P2/PP1P2P1/1RB2K1R b - - 1 20 c0 15098356765531217675; c1 585751677148070980; c2 5501882551812; c3 0",
			"2bq1bkr/rp5p/p5p1/4n3/4N3/2N2P2/PP1P2P1/1RB2K1R w - - 2 21 c0 1263298710249053962; c1 585751677148147780; c2 5781055426052; c3 0",
			"2bq1bkr/rp5p/p5p1/4n3/4N3/2N2P2/PP1P2PR/1RB2K2 b - - 3 21 c0 1263298710249053963; c1 585751677148147780; c2 5785322316676; c3 0",
			"3q1bkr/rp5p/p5p1/4nb2/4N3/2N2P2/PP1P2PR/1RB2K2 w - - 4 22 c0 4690642187817918218; c1 585751677148246788; c2 6064495190916; c3 0",
			"3q1bkr/rp5p/p5p1/4nb2/4N3/P1N2P2/1P1P2PR/1RB2K2 b - - 0 22 c0 4690642187817918219; c1 585503964909449988; c2 6047315321732; c3 0",
			"3q2kr/rp4bp/p5p1/4nb2/4N3/P1N2P2/1P1P2PR/1RB2K2 w - - 1 23 c0 4690732260768947978; c1 585503964909449988; c2 6326488195972; c3 0",
			"3q2kr/rp4bp/p5p1/4nb2/4N3/P1N2P2/1P1P2P1/1RB2K1R b - - 2 23 c0 4690732260768947979; c1 585503964909449988; c2 6330811239940; c3 0",
			"6kr/rp4bp/pq4p1/4nb2/4N3/P1N2P2/1P1P2P1/1RB2K1R w - - 3 24 c0 4841806389942165258; c1 585503964909449988; c2 6609984114180; c3 0",
			"6kr/rp4bp/pq4p1/4nb2/4N2R/P1N2P2/1P1P2P1/1RB2K2 b - - 4 24 c0 4841806389942165259; c1 9368063452825957124; c2 6614251003968; c3 0",
			"3q2kr/rp4bp/p5p1/4nb2/4N2R/P1N2P2/1P1P2P1/1RB2K2 w - - 5 25 c0 4690732260768947978; c1 9368063452825957124; c2 6893423878208; c3 0",
			"3q2kr/rp4bp/p5p1/4nb2/4NR2/P1N2P2/1P1P2P1/1RB2K2 b - - 6 25 c0 4690732260768947979; c1 9368063441551667972; c2 6897718845504; c3 0",
			"3q2kr/rp5p/p5pb/4nb2/4NR2/P1N2P2/1P1P2P1/1RB2K2 w - - 7 26 c0 14128223192016822026; c1 9368063441551667978; c2 7176891719744; c3 0",
			"3q2kr/rp5p/p5pb/4nb2/3PNR2/P1N2P2/1P4P1/1RB2K2 b - - 0 26 c0 14128223192016822027; c1 2315062475156009738; c2 7146826948672; c3 0",
			"3q2kr/rp5p/p5p1/4nb2/3PNb2/P1N2P2/1P4P1/1RB2K2 w - - 0 27 c0 4904851155162046218; c1 144691412213443376; c2 463856553476; c3 0",
			"3q2kr/rp5p/p5p1/4nb2/3PNB2/P1N2P2/1P4P1/1R3K2 b - - 0 27 c0 4904851155162046219; c1 144691403623508784; c2 28991032836; c3 0",
			"3q2kr/rp5p/p5p1/5b2/2nPNB2/P1N2P2/1P4P1/1R3K2 w - - 1 28 c0 4904851155162046218; c1 144691403623809696; c2 30081551876; c3 0",
			"3q2kr/rp5p/p5p1/4Bb2/2nPN3/P1N2P2/1P4P1/1R3K2 b - - 2 28 c0 4904851155162046219; c1 144691401706973776; c2 30098329092; c3 0",
			"3q2kr/rp5p/p5p1/4nb2/3PN3/P1N2P2/1P4P1/1R3K2 w - - 0 29 c0 4904851155162046218; c1 4620729231033772848; c2 1946157280; c3 0",
			"3q2kr/rp5p/p5p1/4Pb2/4N3/P1N2P2/1P4P1/1R3K2 b - - 0 29 c0 4904851155162046219; c1 288795576939588112; c2 121634830; c3 0",
			"6kr/rp5p/p5p1/4Pb2/4N3/P1Nq1P2/1P4P1/1R3K2 w - - 1 30 c0 306553197197532938; c1 288796387360047777; c2 125894670; c3 0",
			"6kr/rp5p/p5p1/4Pb2/4N3/P1Nq1P2/1P4P1/1R2K3 b - - 2 30 c0 306553197197532937; c1 288796387360047777; c2 125960206; c3 0",
			"6kr/rp5p/p5p1/4Pb2/3qN3/P1N2P2/1P4P1/1R2K3 w - - 3 31 c0 306553197197532936; c1 288795576940397217; c2 130220046; c3 0",
			"6kr/rp5p/p5p1/4Pb2/3qN3/P1N2P2/1P4P1/1R3K2 b - - 4 31 c0 306553197197532939; c1 288795576940397217; c2 130285582; c3 0",
			"7r/rp4kp/p5p1/4Pb2/3qN3/P1N2P2/1P4P1/1R3K2 w - - 5 32 c0 306553206722272010; c1 288795576940397217; c2 134545422; c3 0",
			"7r/rp4kp/p5p1/4Pb2/3qN3/P1N2P2/1P4P1/3R1K2 b - - 6 32 c0 306553206722272011; c1 288795576940397217; c2 134611000; c3 0",
			"7r/rp4kp/p5p1/4qb2/4N3/P1N2P2/1P4P1/3R1K2 w - - 0 33 c0 306553206722272010; c1 9241421760413500089; c2 8650755; c3 0",
			"7r/rp4kp/p5p1/4qb2/4N3/P1N2P2/1P3KP1/3R4 b - - 1 33 c0 306553206722272027; c1 13844100579586147001; c2 8654849; c3 0",
			"7r/rp4k1/p5p1/4qb1p/4N3/P1N2P2/1P3KP1/3R4 w - - 0 34 c0 10395452550625106714; c1 13844100579586148523; c2 8912897; c3 0",
			"7r/rp4k1/p5p1/4qb1p/4N3/P1N2P2/1P1R1KP1/8 b - - 1 34 c0 10395452550625106715; c1 159913111820896427; c2 8916992; c3 0",
			"7r/rp4k1/p3b1p1/4q2p/4N3/P1N2P2/1P1R1KP1/8 w - - 2 35 c0 312174459919276826; c1 159913111820896409; c2 9183232; c3 0",
			"7r/rp4k1/p3b1p1/4q2p/4N3/P1N2P2/1P1R2P1/6K1 b - - 3 35 c0 312174459919276813; c1 304028299896752281; c2 9187328; c3 0",
			"7r/rp4k1/p3b3/4q1pp/4N3/P1N2P2/1P1R2P1/6K1 w - - 0 36 c0 10382223226719705868; c1 304028299896752325; c2 9437184; c3 0",
			"7r/rp4k1/p3b3/4q1pp/4N3/P4P2/1P1RN1P1/6K1 b - - 1 36 c0 10382223226719705869; c1 302728545082476741; c2 9441280; c3 0",
			"7r/rp4k1/p3b3/4q2p/4N1p1/P4P2/1P1RN1P1/6K1 w - - 0 37 c0 10382223226719705868; c1 302728545086746761; c2 9699328; c3 0",
			"7r/rp4k1/p3b3/4q2p/4N1p1/P1N2P2/1P1R2P1/6K1 b - - 1 37 c0 10382223226719705869; c1 304028299901022345; c2 9703424; c3 0",
			"5r2/rp4k1/p3b3/4q2p/4N1p1/P1N2P2/1P1R2P1/6K1 w - - 2 38 c0 10382223226701617932; c1 304028299901022345; c2 9969664; c3 0",
			"5r2/rp4k1/p3b3/4q2p/4N1p1/P1NR1P2/1P4P1/6K1 b - - 3 38 c0 10382223226701617933; c1 288795768889028745; c2 9973760; c3 0",
			"5r2/rp4k1/p3b3/4q2p/4N3/P1NR1Pp1/1P4P1/6K1 w - - 0 39 c0 10382223226701617932; c1 288868245569548425; c2 10223616; c3 0",
			"5r2/rp4k1/p2Rb3/4q2p/4N3/P1N2Pp1/1P4P1/6K1 b - - 1 39 c0 95579465322339085; c1 288868233572976793; c2 10227712; c3 0",
			"5r2/rp4k1/3Rb3/p3q2p/4N3/P1N2Pp1/1P4P1/6K1 w - - 0 40 c0 1230951664838253324; c1 288868233572976793; c2 10485760; c3 0",
			"5r2/rp4k1/3Rb3/pN2q2p/4N3/P4Pp1/1P4P1/6K1 b - - 1 40 c0 8148480692479335181; c1 288868230606391696; c2 10489856; c3 0",
			"5r2/1p4k1/r2Rb3/pN2q2p/4N3/P4Pp1/1P4P1/6K1 w - - 2 41 c0 8148482255142787852; c1 288868230606391696; c2 10756096; c3 0",
			"5r2/1p4k1/r2Rb3/pN2q2p/P3N3/5Pp1/1P4P1/6K1 b - - 0 41 c0 8148482255142787853; c1 288868230385207696; c2 10747904; c3 0",
			"2r5/1p4k1/r2Rb3/pN2q2p/P3N3/5Pp1/1P4P1/6K1 w - - 1 42 c0 8148482255137512204; c1 288868230385207696; c2 11014144; c3 0",
			"2r5/1p4k1/r3b3/pN2q2p/P3N3/5Pp1/1P4P1/3R2K1 b - - 2 42 c0 509277775489374989; c1 9241426301253851289; c2 11018243; c3 0",
			"r7/1p4k1/r3b3/pN2q2p/P3N3/5Pp1/1P4P1/3R2K1 w - - 3 43 c0 509277775488809740; c1 9241426301253851289; c2 11284483; c3 0",
			"r7/1p4k1/r3b3/p3q2p/P2NN3/5Pp1/1P4P1/3R2K1 b - - 4 43 c0 10453225752722864909; c1 9241426301253943689; c2 11288579; c3 0",
			"r7/1p3bk1/r7/p3q2p/P2NN3/5Pp1/1P4P1/3R2K1 w - - 5 44 c0 10452880385812658956; c1 9241426301253943689; c2 11554819; c3 0",
			"r7/1p3bk1/r2N4/p3q2p/P2N4/5Pp1/1P4P1/3R2K1 b - - 6 44 c0 1225426961795578637; c1 9241426301252278425; c2 11558915; c3 0",
			"r7/1p3bk1/3r4/p3q2p/P2N4/5Pp1/1P4P1/3R2K1 w - - 0 45 c0 10453057407184730892; c1 4036353657648808329; c2 737280; c3 0",
			"r7/1p3bk1/3r4/p3qN1p/P7/5Pp1/1P4P1/3R2K1 b - - 1 45 c0 10453057407184730893; c1 4036353657648715911; c2 737536; c3 0",
			"r7/1p3bk1/3r4/p4q1p/P7/5Pp1/1P4P1/3R2K1 w - - 0 46 c0 2382606874936802060; c1 252272103603044747; c2 47104; c3 0",
			"r7/1p3bk1/3r4/p4q1p/P7/5Pp1/1P4P1/2R3K1 b - - 1 46 c0 2382606874936802061; c1 126171314036670859; c2 47120; c3 0",
			"r1b1kbr1/pppq1p1p/2n2p2/3p4/3P4/2N2N1P/PPP1PPP1/R2QKB1R b KQq - 1 8 c0 11730860075175837193; c1 603693490802664648; c2 1557212510029939745; c3 32",
			"r1bnkbr1/pppq1p1p/5p2/3p4/3P4/2N2N1P/PPP1PPP1/R2QKB1R w KQq - 2 9 c0 3226320465868553736; c1 603693490802663562; c2 2710134014636786721; c3 36",
			"r1bnkbr1/pppq1p1p/5p2/3p4/3P4/2N1PN1P/PPP2PP1/R2QKB1R b KQq - 0 9 c0 3226320465868553737; c1 9660432341444731018; c2 404291005423043088; c3 36",
			"r1bnk1r1/pppq1p1p/5p2/3p4/1b1P4/2N1PN1P/PPP2PP1/R2QKB1R w KQq - 1 10 c0 11730860074789961224; c1 9660432352585196040; c2 1557212510029890064; c3 40",
			"r1bnk1r1/pppq1p1p/5p2/3p4/1b1P4/2NQPN1P/PPP2PP1/R3KB1R b KQq - 2 10 c0 11730860074789961225; c1 7010063971877659144; c2 2710129518844322056; c3 40",
			"r1bnk1r1/pppq1p1p/5p2/b2p4/3P4/2NQPN1P/PPP2PP1/R3KB1R w KQq - 3 11 c0 11730860074789961224; c1 7010063960738439688; c2 3863051023451169032; c3 44",
			"r1bnk1r1/pppq1p1p/5p2/b2p4/1P1P4/2NQPN1P/P1P2PP1/R3KB1R b KQq - 0 11 c0 11730860074789961225; c1 1480558935987790344; c2 404286509630623878; c3 44",
			"r1bnk1r1/pppq1p1p/5p2/3p4/1b1P4/2NQPN1P/P1P2PP1/R3KB1R w KQq - 0 12 c0 11730860074789961224; c1 7010063971877659144; c2 25267906851913992; c3 3",
			"r1bnk1r1/pppq1p1p/5p2/3p4/1b1P4/2NQPN1P/P1P2PP1/2KR1B1R b q - 1 12 c0 11730860074789961221; c1 7010063971877659144; c2 90570191643099400; c3 3",
			"r1bnk3/pppq1p1p/5pr1/3p4/1b1P4/2NQPN1P/P1P2PP1/2KR1B1R w q - 2 13 c0 9956550790294994436; c1 7010063971877674528; c2 4774313804108415240; c3 3",
			"r1bnk3/pppq1p1p/5pr1/3p4/1b1P2P1/2NQPN1P/P1P2P2/2KR1B1R b q - 0 13 c0 9956550790294994437; c1 1480562821187526176; c2 4630198615997157510; c3 3",
			"r1bnk3/pppq1p1p/6r1/3p1p2/1b1P2P1/2NQPN1P/P1P2P2/2KR1B1R w q - 0 14 c0 9956550790294994436; c1 1480562821214799296; c2 9241884634424545414; c3 3",
			"r1bnk3/pppq1p1p/6r1/3pNp2/1b1P2P1/2NQP2P/P1P2P2/2KR1B1R b q - 1 14 c0 9956550790294994437; c1 5242261065722004928; c2 9313942228462473345; c3 3",
			"r1bnk3/ppp2p1p/4q1r1/3pNp2/1b1P2P1/2NQP2P/P1P2P2/2KR1B1R w q - 2 15 c0 622136875127864836; c1 5242261065722004953; c2 13997685840927789185; c3 3",
			"r1bnk3/ppp2p1p/4q1N1/3p1p2/1b1P2P1/2NQP2P/P1P2P2/2KR1B1R b q - 0 15 c0 622136875127864837; c1 1480562821214798041; c2 4324612679623786760; c3 0",
			"r1bnk3/ppp2p2/4q1p1/3p1p2/1b1P2P1/2NQP2P/P1P2P2/2KR1B1R w q - 0 16 c0 10385940867267100164; c1 9315907213180700741; c2 288302690985968656; c3 0",
			"r1bnk3/ppp2p2/4q1p1/1Q1p1p2/1b1P2P1/2N1P2P/P1P2P2/2KR1B1R b q - 1 16 c0 10385940867267100165; c1 9298811774329983557; c2 288584165962679312; c3 0",
			"r1b1k3/ppp2p2/2n1q1p1/1Q1p1p2/1b1P2P1/2N1P2P/P1P2P2/2KR1B1R w q - 2 17 c0 10719170070995205636; c1 9298811774329983557; c2 306880039448871952; c3 0",
			"r1b1k3/ppp2p2/2n1q1p1/1Q1N1p2/1b1P2P1/4P2P/P1P2P2/2KR1B1R b q - 0 17 c0 10719170070995205637; c1 580968795647087173; c2 19144818093465665; c3 0",
			"r1b1k3/1pp2p2/p1n1q1p1/1Q1N1p2/1b1P2P1/4P2P/P1P2P2/2KR1B1R w q - 0 18 c0 10723389872336993796; c1 580968795647087173; c2 20270718000308289; c3 0",
			"r1b1k3/1pN2p2/p1n1q1p1/1Q3p2/1b1P2P1/4P2P/P1P2P2/2KR1B1R b q - 0 18 c0 10723389631818825221; c1 1189232054334788165; c2 1266919875019268; c3 0",
			"r1b5/1pNk1p2/p1n1q1p1/1Q3p2/1b1P2P1/4P2P/P1P2P2/2KR1B1R w - - 1 19 c0 10723389692485237124; c1 1189232054334788165; c2 1338113252917764; c3 0",
			"r1b5/1pNk1p2/pQn1q1p1/5p2/1b1P2P1/4P2P/P1P2P2/2KR1B1R b - - 2 19 c0 5562264519518648709; c1 1189232054334784601; c2 1339212764545540; c3 0",
			"r1b5/1pNk1p2/pQn3p1/5p2/1b1P2P1/4P2P/q1P2P2/2KR1B1R w - - 0 20 c0 5562264519518648708; c1 6415395278733582404; c2 87961405341728; c3 0",
			"r1b5/1pNk1p2/p1n3p1/5p2/1Q1P2P1/4P2P/q1P2P2/2KR1B1R b - - 0 20 c0 4958782169451002245; c1 400962204919276036; c2 5497587833858; c3 0",
			"r1b5/1pNk1p2/p5p1/5p2/1n1P2P1/4P2P/q1P2P2/2KR1B1R w - - 0 21 c0 4904738973922556292; c1 2330903147021230624; c2 360779108800; c3 0",
			"r1b5/1pNk1p2/p5p1/5p2/1n1P2P1/3BP2P/q1P2P2/2KR3R b - - 1 21 c0 4904738973922556293; c1 400962375702438432; c2 361047530498; c3 0",
			"r1b5/1pk2p2/p5p1/5p2/1n1P2P1/3BP2P/q1P2P2/2KR3R w - - 0 22 c0 306546196550973700; c1 2330903157695096354; c2 23622435264; c3 0",
			"r1b5/1pk2p2/p5p1/5p2/1n1P2P1/3BP2P/q1P2P2/2KR2R1 b - - 1 22 c0 306546196550973701; c1 2330903157695096354; c2 23639155136; c3 0",
			"r1b5/1pk2p2/p5p1/8/1n1P1pP1/3BP2P/q1P2P2/2KR2R1 w - - 0 23 c0 306546196550973700; c1 2330903157697284704; c2 24696119744; c3 0",
			"r1b5/1pk2p2/p5p1/8/1n1P1pP1/3BP2P/q1PK1P2/3R2R1 b - - 1 23 c0 306546196550973719; c1 1177981653090437728; c2 24712896960; c3 0",
			"r1b5/1pk2p2/p1n3p1/8/3P1pP1/3BP2P/q1PK1P2/3R2R1 w - - 2 24 c0 4921609914698889494; c1 1177981653090437120; c2 25803416000; c3 0",
			"r1b5/1pk2p2/p1n3p1/8/3PBpP1/4P2P/q1PK1P2/3R2R1 b - - 3 24 c0 4921609914698889495; c1 1177981642449684480; c2 25820193216; c3 0",
			"r1b5/1pk2p2/p1n3p1/8/3PB1P1/4p2P/q1PK1P2/3R2R1 w - - 0 25 c0 4921609914698889494; c1 73623921370662912; c2 1677725212; c3 0",
			"r1b5/1pk2p2/p1n3p1/8/3PB1P1/4P2P/q1PK4/3R2R1 b - - 0 25 c0 4921609914698889495; c1 13836624313895421952; c2 104857825; c3 0",
			"r1b5/1pk1np2/p5p1/8/3PB1P1/4P2P/q1PK4/3R2R1 w - - 1 26 c0 4904740176513399062; c1 13836624313895421952; c2 109117665; c3 0",
			"r1b5/1pk1np2/p5p1/8/3PB1P1/4P2P/q1PK4/3R1R2 b - - 2 26 c0 4904740176513399063; c1 13836624313895421952; c2 109183089; c3 0",
			"r1b5/1pk1np2/p3q1p1/8/3PB1P1/4P2P/2PK4/3R1R2 w - - 3 27 c0 6417949651309885718; c1 13836192820073889796; c2 113442929; c3 0",
			"r1b5/1pk1np2/p3q1p1/8/3P2P1/4PB1P/2PK4/3R1R2 b - - 4 27 c0 6417949651309885719; c1 13836193442775990276; c2 113508465; c3 0",
			"r1b5/1pk1n3/p3q1p1/5p2/3P2P1/4PB1P/2PK4/3R1R2 w - - 0 28 c0 5012806776360990998; c1 13836193442775990816; c2 117440625; c3 0",
			"r1b5/1pk1n3/p3q1p1/5p2/3P2P1/4PB1P/2PK4/3R2R1 b - - 1 28 c0 5012806776360990999; c1 13836193442775990816; c2 117506273; c3 0",
			"rkb5/1p2n3/p3q1p1/5p2/3P2P1/4PB1P/2PK4/3R2R1 w - - 2 29 c0 5012806767228681366; c1 13836193442775990816; c2 121766113; c3 0",
			"rkb5/1p2n3/p3q1p1/5p2/3P2P1/4PB1P/2PK4/1R4R1 b - - 3 29 c0 5012806767228681367; c1 8071585919741755936; c2 121831648; c3 0",
			"1kb5/rp2n3/p3q1p1/5p2/3P2P1/4PB1P/2PK4/1R4R1 w - - 4 30 c0 5012806767609207958; c1 8071585919741755936; c2 126091488; c3 0",
			"1kb5/rp2n3/p3q1p1/5p2/3P2P1/4PB1P/2PK4/1R1R4 b - - 5 30 c0 5012806767609207959; c1 8071585919741755936; c2 126156828; c3 0",
			"1kb5/rp2n3/p2q2p1/5p2/3P2P1/4PB1P/2PK4/1R1R4 w - - 6 31 c0 4956511772267076758; c1 8071585919741755936; c2 130416668; c3 0",
			"1kb5/rp2n3/p2q2p1/5p2/3P2P1/4PB1P/2P5/1RKR4 b - - 7 31 c0 4956511772267076741; c1 16142036451989684768; c2 130482204; c3 0",
			"1kb5/rp2n3/p5p1/5p2/3P2P1/4PBqP/2P5/1RKR4 w - - 8 32 c0 306545157007039620; c1 16142042675661768738; c2 134742044; c3 0",
			"1kb5/rp2n3/p5p1/5p2/3P2P1/4P1qP/2P1B3/1RKR4 b - - 9 32 c0 306545157007039621; c1 16163490410898130978; c2 134807580; c3 0",
			"1kb5/rp2n3/p5p1/5p2/3P2P1/4q2P/2P1B3/1RKR4 w - - 0 33 c0 306545157007039620; c1 14845276185814108194; c2 8650753; c3 0",
			"1kb5/rp2n3/p5p1/5p2/3P2P1/4q2P/1KP1B3/1R1R4 b - - 1 33 c0 306545157007039635; c1 14340167141083580450; c2 8654849; c3 0",
			"1kb5/rp2n3/p5p1/5p2/3P2P1/7P/1KP1q3/1R1R4 w - - 0 34 c0 306545157007039634; c1 2049357872365832226; c2 557056; c3 0",
			"1kb5/rp2n3/p5p1/5p2/3P2P1/3R3P/1KP1q3/1R6 b - - 1 34 c0 306545157007039635; c1 507923829797160994; c2 557312; c3 0",
			"1kb5/rp2n3/p5p1/8/3P1pP1/3R3P/1KP1q3/1R6 w - - 0 35 c0 306545157007039634; c1 507923829797298304; c2 573440; c3 0",
			"1kb5/rp2n3/p5p1/8/3P1pP1/3R3P/1KP1q3/6R1 b - - 1 35 c0 306545157007039635; c1 16144421736027660416; c2 573696; c3 0",
			"1kb5/rp2n3/p3q1p1/8/3P1pP1/3R3P/1KP5/6R1 w - - 2 36 c0 5012806767609207954; c1 16140936813662570496; c2 590336; c3 0",
			"1kb5/rp2n3/p3q1p1/8/3P1pPP/3R4/1KP5/6R1 b - - 0 36 c0 5012806767609207955; c1 16140936489526757376; c2 589824; c3 0",
			"1kb5/rp2n3/p4qp1/8/3P1pPP/3R4/1KP5/6R1 w - - 1 37 c0 5125396758293470354; c1 16140936489526757376; c2 606464; c3 0",
			"1kb5/rp2n3/p4qp1/6P1/3P1p1P/3R4/1KP5/6R1 b - - 0 37 c0 5125396758293470355; c1 16140936489556017216; c2 606208; c3 0",
			"1kb4q/rp2n3/p5p1/6P1/3P1p1P/3R4/1KP5/6R1 w - - 1 38 c0 4904722512317078674; c1 16140936489556017216; c2 622848; c3 0",
			"1kb4q/rp2n3/p5p1/6P1/3P1p1P/8/1KP5/3R2R1 b - - 2 38 c0 4904722512317078675; c1 16267004053255389248; c2 623104; c3 0",
			"1kb4q/rp6/p5p1/5nP1/3P1p1P/8/1KP5/3R2R1 w - - 3 39 c0 306543994236918930; c1 16267004053255389286; c2 639744; c3 0",
			"1kb4q/rp6/p5p1/5nPP/3P1p2/8/1KP5/3R2R1 b - - 0 39 c0 306543994236918931; c1 16267004053656438886; c2 638976; c3 0",
			"1kb5/rp6/p5p1/5nPq/3P1p2/8/1KP5/3R2R1 w - - 0 40 c0 6936688027268111506; c1 1016687753353530502; c2 40960; c3 0",
			"1kb5/rp6/p5p1/3P1nPq/5p2/8/1KP5/3R2R1 b - - 0 40 c0 595619751930453139; c1 1016687753353545830; c2 40960; c3 0",
			"1kb5/rp6/p5p1/3P2Pq/5p2/4n3/1KP5/3R2R1 w - - 1 41 c0 595619751930453138; c1 1016687758420348036; c2 42000; c3 0",
			"1kb5/rp6/p5p1/3P2Pq/5p2/4n3/1KPR4/6R1 b - - 2 41 c0 595619751930453139; c1 1008837245398027396; c2 42016; c3 0",
			"1kb5/rp6/p5p1/3P2P1/5p2/4nq2/1KPR4/6R1 w - - 3 42 c0 595619751930453138; c1 1008837254036201476; c2 43056; c3 0",
			"1kb5/rp6/p5p1/3P2P1/5p2/4nq2/2PR4/2K3R1 b - - 4 42 c0 595619751930453125; c1 1008868177800732676; c2 43072; c3 0",
			"1k6/rp1b4/p5p1/3P2P1/5p2/4nq2/2PR4/2K3R1 w - - 5 43 c0 595619788109126788; c1 1008868177800732676; c2 44112; c3 0",
			"1k6/rp1b4/p2P2p1/6P1/5p2/4nq2/2PR4/2K3R1 b - - 0 43 c0 306404249538927749; c1 1008868177800732676; c2 44032; c3 0",
			"1k6/rp1b4/p2P2p1/6P1/4qp2/4n3/2PR4/2K3R1 w - - 1 44 c0 306404249538927748; c1 1008868169162657796; c2 45072; c3 0",
			"1k6/rp1b4/p2P2p1/6P1/4qp2/4n3/2PR4/1K4R1 b - - 2 44 c0 306404249538927747; c1 1008868169162657796; c2 45088; c3 0",
			"1k6/rp1b4/p2P2p1/5qP1/5p2/4n3/2PR4/1K4R1 w - - 3 45 c0 306404249538927746; c1 1008868169162555506; c2 46128; c3 0",
			"1k6/rp1b4/p2P2p1/5qP1/5p2/4n3/2PR4/2K3R1 b - - 4 45 c0 306404249538927749; c1 1008868169162555506; c2 46144; c3 0",
			"1k6/rp1b4/p2P2p1/q5P1/5p2/4n3/2PR4/2K3R1 w - - 5 46 c0 10682697791000550532; c1 1008868169162555457; c2 47184; c3 0",
			"1k6/rp1b4/p2P2p1/q5P1/5p2/4n3/2P3R1/2K3R1 b - - 6 46 c0 10682697791000550533; c1 1009299177720643649; c2 47200; c3 0",
			"1k6/rp6/p1bP2p1/q5P1/5p2/4n3/2P3R1/2K3R1 w - - 7 47 c0 10682772581413559428; c1 1009299177720643649; c2 48240; c3 0",
			"1k6/rp6/p1bP2p1/q5P1/5pR1/4n3/2P5/2K3R1 b - - 8 47 c0 10682772581413559429; c1 1008810796213469249; c2 48256; c3 0",
			"1k6/rp6/p1bP2p1/6P1/5pR1/4n3/2P5/q1K3R1 w - - 9 48 c0 306479039951936644; c1 1015843470928912388; c2 49296; c3 0",
			"1k6/rp6/p1bP2p1/6P1/5pR1/4n3/2PK4/q5R1 b - - 10 48 c0 306479039951936663; c1 1012325033720029188; c2 49312; c3 0",
			"1k6/rp6/p1bP2p1/q5P1/5pR1/4n3/2PK4/6R1 w - - 11 49 c0 10682772581413559446; c1 1008810796213469249; c2 50352; c3 0",
			"1k6/rp6/p1bP2p1/q5P1/5pR1/2P1n3/3K4/6R1 b - - 0 49 c0 10682772581413559447; c1 1008807623306379329; c2 50176; c3 0",
			"1k6/rp6/p1bP2p1/6P1/5pR1/2P1n3/q2K4/6R1 w - - 1 50 c0 306479039951936662; c1 1008833885995147268; c2 51216; c3 0",
			"1k6/rp6/p1bP2p1/6P1/5pR1/2P1n3/q7/4K1R1 b - - 2 50 c0 306479039951936649; c1 1008833885995147268; c2 51232; c3 0",
			"1k6/rp6/p1bP2p1/6P1/6R1/2P1np2/q7/4K1R1 w - - 0 51 c0 306479039951936648; c1 1008833955455156228; c2 52224; c3 0",
			"1k6/rp6/p1bP2p1/6P1/8/2P1np2/q5R1/4K1R1 b - - 1 51 c0 306479039951936649; c1 1009300625172987908; c2 52240; c3 0",
			"1k6/rp6/p1bP2p1/6P1/8/2P2p2/q5n1/4K1R1 w - - 0 52 c0 306479039951936648; c1 63134065611767812; c2 3328; c3 0",
			"1k6/rp6/p1bP2p1/6P1/8/2P2p2/q5R1/4K3 b - - 0 52 c0 306479039951936649; c1 30894270447620; c2 208; c3 0",
			"1k6/rp6/p1bP2p1/6P1/8/2P2p2/6q1/4K3 w - - 0 53 c0 306479039951936648; c1 4611692890945748996; c2 13; c3 0",
			"1k6/rp6/p1bP2p1/6P1/8/2P2p2/6q1/3K4 b - - 1 53 c0 306479039951936647; c1 4683750484983676932; c2 13; c3 0",
			"rnbqkb2/ppppp1p1/5B2/7r/3P3p/N1P1pP2/PP2K1PP/R2Q1BNR b q - 1 8 c0 1785168365649591833; c1 1171929899074625605; c2 4739566107787010307; c3 513",
			"rnbqkb2/ppppp3/5p2/7r/3P3p/N1P1pP2/PP2K1PP/R2Q1BNR w q - 0 9 c0 1785168365649591832; c1 3532010132512711169; c2 296222881736688144; c3 36",
			"rnbqkb2/ppppp3/5p2/7r/3P3p/N1P1KP2/PP4PP/R2Q1BNR b q - 0 9 c0 1785168365649591849; c1 9296617655546946049; c2 4630199948535930896; c3 2",
			"rnbqkb2/ppppp3/8/5p1r/3P3p/N1P1KP2/PP4PP/R2Q1BNR w q - 0 10 c0 1785168365649591848; c1 9296617655549165569; c2 9241885966963318800; c3 2",
			"rnbqkb2/ppppp3/8/5p1r/3P1P1p/N1P1K3/PP4PP/R2Q1BNR b q - 0 10 c0 1785168365649591849; c1 9242380533657370625; c2 9241885966963318800; c3 2",
			"r1b1kb1r/pp2ppp1/n1p3qn/3pP2p/3P3P/1NP5/PP2NPP1/R1BQKB1R b KQkq - 2 8 c0 5057862458248199689; c1 6953707914584278099; c2 14215971781864460292; c3 131705",
			"r1b1kb1r/ppn1ppp1/2p3qn/3pP2p/3P3P/1NP5/PP2NPP1/R1BQKB1R w KQkq - 3 9 c0 7139581017256033800; c1 6953707914584278084; c2 14215971781864460292; c3 148345",
			"r1b1kb1r/ppn1ppp1/2p3qn/3pP2p/3P3P/1NP3N1/PP3PP1/R1BQKB1R b KQkq - 4 9 c0 7139581017256033801; c1 6953707914584278084; c2 14215971781816159236; c3 148601",
			"r3kb1r/ppn1ppp1/2p3qn/3pP2p/3P2bP/1NP3N1/PP3PP1/R1BQKB1R w KQkq - 5 10 c0 5057909832005385736; c1 6977211103324005444; c2 14215971781816159236; c3 165241",
			"r3kb1r/ppn1ppp1/2p3qn/3pP2p/3P2bP/1NP3N1/PP1Q1PP1/R1B1KB1R b KQkq - 6 10 c0 5057909832005385737; c1 6977211103324005444; c2 14201098954986294276; c3 165497",
			"1r2kb1r/ppn1ppp1/2p3qn/3pP2p/3P2bP/1NP3N1/PP1Q1PP1/R1B1KB1R w KQk - 7 11 c0 5057909832005574152; c1 6977211103324005444; c2 14201098954986294276; c3 182073",
			"1r2kb1r/ppn1ppp1/2p3qn/3pP2p/3P2bP/1NP3N1/PP1Q1PP1/1RB1KB1R b Kk - 8 11 c0 5057909832005574153; c1 6977211103324005444; c2 14201222100288605188; c3 182313",
			"1r2kb1r/ppn1ppp1/2p4n/3pP2p/3P2bP/1NP3N1/PP1Q1PP1/1qB1KB1R w Kk - 0 12 c0 5057909832005574152; c1 5047761712385136708; c2 11263909505148260544; c3 12290",
			"1r2kb1r/ppn1ppp1/2p4n/3pP2p/3P2bP/PNP3N1/1P1Q1PP1/1qB1KB1R b Kk - 0 12 c0 5057909832005574153; c1 6993316751409190980; c2 11263909505148259332; c3 12290",
			"1r2kb1r/ppn1ppp1/2p4n/3pPb1p/3P3P/PNP3N1/1P1Q1PP1/1qB1KB1R w Kk - 1 13 c0 5057909832005574152; c1 6991847813001287748; c2 11263909505148259332; c3 13330",
			"1r2kb1r/ppn1ppp1/2p4n/3pPN1p/3P3P/PNP5/1P1Q1PP1/1qB1KB1R b Kk - 0 13 c0 5057909832005574153; c1 6991847803337611332; c2 3009837353285459972; c3 832",
			"1r2kb1r/ppn1ppp1/2p4n/3pPN1p/3P3P/PNP5/qP1Q1PP1/2B1KB1R w Kk - 1 14 c0 5057909832005574152; c1 6991847803337611332; c2 3009833933992071172; c3 897",
			"1r2kb1r/ppn1ppN1/2p4n/3pP2p/3P3P/PNP5/qP1Q1PP1/2B1KB1R b Kk - 0 14 c0 1022684565881609737; c1 5048676506059446340; c2 188114620874504448; c3 56",
			"1r1k1b1r/ppn1ppN1/2p4n/3pP2p/3P3P/PNP5/qP1Q1PP1/2B1KB1R w K - 1 15 c0 1022684565881609608; c1 5048676506059446340; c2 1196920937405495552; c3 60",
			"1r1k1b1r/ppn1pp2/2p4n/3pP2N/3P3P/PNP5/qP1Q1PP1/2B1KB1R b K - 0 15 c0 4769679455853862281; c1 315542279749667204; c2 13837808019832079248; c3 3",
			"1r1k1b1r/ppn1pp2/2p4n/3pP2N/3P3P/PqP5/1P1Q1PP1/2B1KB1R w K - 0 16 c0 4769679455853862280; c1 513700663353969028; c2 4611857891211757600; c3 0",
			"1r1k1b1r/ppn1pp2/2p4n/3pP2N/3P3P/PqPB4/1P1Q1PP1/2B1K2R b K - 1 16 c0 4769679455853862281; c1 9737072700208744836; c2 4616361227837538818; c3 0",
			"1r1k1b1r/ppn1pp2/2p5/3pPn1N/3P3P/PqPB4/1P1Q1PP1/2B1K2R w K - 2 17 c0 4769679455853862280; c1 9737072700247083012; c2 4909095203616621058; c3 0",
			"1r1k1b1r/ppn1pp2/2p5/3pPn1N/3P2PP/PqPB4/1P1Q1P2/2B1K2R b K - 0 17 c0 4769679455853862281; c1 8219214490288359428; c2 4900088004330659880; c3 0",
			"1r1k1b1r/ppn2p2/2p1p3/3pPn1N/3P2PP/PqPB4/1P1Q1P2/2B1K2R w K - 0 18 c0 4909572519279058312; c1 8219214490288359441; c2 5188318380482371624; c3 0",
			"1r1k1b1r/ppn2p2/2p1p3/3pPn1N/3P2PP/PqPB4/1P2QP2/2B1K2R b K - 1 18 c0 4909572519279058313; c1 8219214490288359441; c2 5192821980114460712; c3 0",
			"1r1k1b2/ppn2p2/2p1p3/3pPn1r/3P2PP/PqPB4/1P2QP2/2B1K2R w K - 0 19 c0 1459769786910498184; c1 9737072942665570433; c2 342284297289925122; c3 0",
			"1r1k1b2/ppn2p2/2p1p3/3pPn1P/3P3P/PqPB4/1P2QP2/2B1K2R b K - 0 19 c0 1459769786910498185; c1 2914410052962359425; c2 21392768580620320; c3 0",
			"1r3b2/ppnk1p2/2p1p3/3pPn1P/3P3P/PqPB4/1P2QP2/2B1K2R w K - 1 20 c0 1459775160227060104; c1 2914410052962359425; c2 22536260673507360; c3 0",
			"1r3b2/ppnk1p2/2p1p3/3pPB1P/3P3P/PqP5/1P2QP2/2B1K2R b K - 0 20 c0 1459775160227060105; c1 32106291443406977; c2 1407416780466434; c3 0",
			"1r3b2/ppnk1p2/2p5/3pPp1P/3P3P/PqP5/1P2QP2/2B1K2R w K - 0 21 c0 306853655620213128; c1 2307849652429005192; c2 92361595290256; c3 0",
			"1r3b2/ppnk1p2/2p4P/3pPp2/3P3P/PqP5/1P2QP2/2B1K2R b K - 0 21 c0 9530225692474988937; c1 2307849652430575744; c2 92361595290256; c3 0",
			"1r3b2/pp1k1p2/2p1n2P/3pPp2/3P3P/PqP5/1P2QP2/2B1K2R w K - 1 22 c0 10611644306660120968; c1 2307849652430575744; c2 96828361278096; c3 0",
			"1r3b2/pp1k1p2/2p1n2P/3pPp2/3P3P/PqPQ4/1P3P2/2B1K2R b K - 2 22 c0 10611644306660120969; c1 326265816387557504; c2 96897080754690; c3 0",
			"1r3b2/pp1k1p2/2p1n2P/3pP3/3P1p1P/PqPQ4/1P3P2/2B1K2R w K - 0 23 c0 10611644306660120968; c1 326265825386305664; c2 101157688312322; c3 0",
			"1r3b2/pp1k1p2/2p1n2P/3pPQ2/3P1p1P/PqP5/1P3P2/2B1K2R b K - 1 23 c0 10611644306660120969; c1 32106435451164800; c2 101226407789058; c3 0",
			"1r6/pp1kbp2/2p1n2P/3pPQ2/3P1p1P/PqP5/1P3P2/2B1K2R w K - 2 24 c0 10611645467320506760; c1 32106435451164800; c2 105693173776898; c3 0",
			"1r6/pp1kbp2/2p1n2P/3pPQ2/3P1p1P/PqP5/1P3P2/2B1K1R1 b - - 3 24 c0 10611645467320506761; c1 32106435451164800; c2 105759510888962; c3 0",
			"1r6/pp1kbp2/2p1n2P/3pPQ2/3P1p1P/P1P5/qP3P2/2B1K1R1 w - - 4 25 c0 10611645467320506760; c1 10377437729474615424; c2 110226276876803; c3 0",
			"1r6/pp1kbp1P/2p1n3/3pPQ2/3P1p1P/P1P5/qP3P2/2B1K1R1 b - - 0 25 c0 3765188871298865545; c1 10377437729474615425; c2 109951398969859; c3 0",
			"1r6/pp1kbp1P/4n3/2ppPQ2/3P1p1P/P1P5/qP3P2/2B1K1R1 w - - 0 26 c0 1369273869537761672; c1 10377437729474615492; c2 114349445480963; c3 0",
			"1r4R1/pp1kbp1P/4n3/2ppPQ2/3P1p1P/P1P5/qP3P2/2B1K3 b - - 1 26 c0 3461637838947604873; c1 18307008207883329; c2 114417949745209; c3 0",
			"1r4R1/pp1kbp1P/4n3/2ppPQ2/3P1p1P/P1P5/1P3P2/q1B1K3 w - - 2 27 c0 3461637838947604872; c1 18307008207883329; c2 118884717363714; c3 0",
			"1r4R1/pp1kbp1P/4n3/2ppPQ2/3P1p1P/P1P5/1P1K1P2/q1B5 b - - 3 27 c0 3461637838947604887; c1 18307008207883329; c2 118953425535234; c3 0",
			"1r4R1/pp1kbp1P/4n3/2ppPQ2/3P1p1P/P1P5/1P1K1P2/2q5 w - - 0 28 c0 3461637838947604886; c1 18307008207883329; c2 7696584671490; c3 0",
			"1r4R1/pp1kbp1P/4n3/2ppPQ2/3P1p1P/P1P5/1P3P2/2K5 b - - 0 28 c0 3461637838947604869; c1 18307008207883329; c2 481036337666; c3 0",
			"1r4R1/pp1kbp1P/4n3/3pPQ2/3p1p1P/P1P5/1P3P2/2K5 w - - 0 29 c0 3461637838947604868; c1 2306987199374170241; c2 31138512928; c3 0",
			"1r4RQ/pp1kbp2/4n3/3pPQ2/3p1p1P/P1P5/1P3P2/2K5 b - - 0 29 c0 3468708920363964805; c1 2306987199374170241; c2 31138512928; c3 0",
			"5rRQ/pp1kbp2/4n3/3pPQ2/3p1p1P/P1P5/1P3P2/2K5 w - - 1 30 c0 3468708920369617284; c1 2306987199374170241; c2 32229031968; c3 0",
			"5R1Q/pp1kbp2/4n3/3pPQ2/3p1p1P/P1P5/1P3P2/2K5 b - - 0 30 c0 1369715812127742341; c1 144186699960885640; c2 2013265922; c3 0",
			"5R1Q/pp1kbp2/4n3/3pPQ2/5p1P/P1p5/1P3P2/2K5 w - - 0 31 c0 1369715812127742340; c1 2314925046696583560; c2 130023424; c3 0",
			"1R5Q/pp1kbp2/4n3/3pPQ2/5p1P/P1p5/1P3P2/2K5 b - - 1 31 c0 1369715812126022021; c1 2314925046696583560; c2 130088960; c3 0",
			"1R5Q/pp1kbp2/4n3/3pPQ2/5p1P/P7/1p3P2/2K5 w - - 0 32 c0 1369715812126022020; c1 153685408721478024; c2 8388608; c3 0",
			"1R5Q/pp1kbp2/4n3/3pPQ2/5p1P/P7/1K3P2/8 b - - 0 32 c0 1369715812126022035; c1 4503671064830344; c2 524288; c3 0",
			"1R3b1Q/pp1k1p2/4n3/3pPQ2/5p1P/P7/1K3P2/8 w - - 1 33 c0 1369697242503567762; c1 4503671064830344; c2 540928; c3 0",
			"1R3b1Q/pp1k1Q2/4n3/3pP3/5p1P/P7/1K3P2/8 b - - 0 33 c0 1369415767526857107; c1 281479441547656; c2 33792; c3 0",
			"1R3b1Q/pp3Q2/2k1n3/3pP3/5p1P/P7/1K3P2/8 w - - 1 34 c0 1369732426875655442; c1 281479441547656; c2 34832; c3 0",
			"1R3b1Q/pQ6/2k1n3/3pP3/5p1P/P7/1K3P2/8 b - - 0 34 c0 9308943052208985363; c1 17592465096728; c2 2176; c3 0",
			"1R3b1Q/pQ6/4n3/2kpP3/5p1P/P7/1K3P2/8 w - - 1 35 c0 9394511445129023762; c1 17592465096728; c2 2241; c3 0",
			"1RQ2b1Q/p7/4n3/2kpP3/5p1P/P7/1K3P2/8 b - - 2 35 c0 9394511199225893139; c1 17592465096728; c2 2242; c3 0",
			"1RQ2b1Q/p1n5/8/2kpP3/5p1P/P7/1K3P2/8 w - - 3 36 c0 9223541539153236242; c1 17592465096728; c2 2307; c3 0",
			"1R3b1Q/p1Q5/8/2kpP3/5p1P/P7/1K3P2/8 b - - 0 36 c0 9799837885493530899; c1 1099529068545; c2 144; c3 0",
			"1R3b1Q/p1Q5/8/3pP3/3k1p1P/P7/1K3P2/8 w - - 1 37 c0 1152926600942177682; c1 1152922604135915523; c2 148; c3 0;"
		};
		for (std::string line : testData) {
			const size_t indx = line.find(" c0");
			std::string fen = line.substr(0, indx);
			auto token = utils::split(fen);
			int move_count = 1;
			if (token.size() >= 6)
				move_count = std::stoi(token.at(5));
			Position pos(fen);
			const size_t indx2 = line.find("; c1 ", indx + 4);
			uint64_t u1 = std::stoull(line.substr(indx + 3, indx2 - indx - 3));
			const size_t indx3 = line.find("; c2 ", indx2 + 4);
			uint64_t u2 = std::stoull(line.substr(indx2 + 4, indx3 - indx2 - 4));
			const size_t indx4 = line.find("; c3 ", indx3 + 4);
			uint64_t u3 = std::stoull(line.substr(indx3 + 4, indx4 - indx3 - 4));
			uint64_t u4 = std::stoull(line.substr(indx4 + 4));
			uint8_t data[32];
			pos.pack(data, move_count);
			union t_convert
			{
				uint64_t bigvar;
				uint8_t  charvar[8];
			}
			convert;
			memcpy(convert.charvar, &data[0], 8);
			if (convert.bigvar != u1)
				return false;
			memcpy(convert.charvar, &data[8], 8);
			if (convert.bigvar != u2)
				return false;
			memcpy(convert.charvar, &data[16], 8);
			if (convert.bigvar != u3)
				return false;
			memcpy(convert.charvar, &data[24], 8);
			if (convert.bigvar != u4)
				return false;
			Position pos2(data);
			if (pos.GetHash() != pos2.GetHash())
				return false;
		}

		return true;
	}

	uint64_t nnue_evaluate(std::string ifile, std::string ofile, std::string eval_file)
	{
		settings::parameter.NNUEFile = eval_file;
		Initialize();
		settings::parameter.UseNNUE = network.Load(settings::parameter.NNUEFile);
		if (!settings::parameter.UseNNUE) {
			std::cout << "Eval File " << settings::parameter.NNUEFile << " not found!" << std::endl;
			exit(-1);
		}
		uint64_t count = 0;
		std::ifstream input;
		input.open(ifile);
		std::string l;
		if (!input.is_open()) {
			std::cout << "Couldn't open " << settings::parameter.NNUEFile << std::endl;
			exit(-1);
		}
		std::ofstream of;
		of.open(ofile);
		if (!of.is_open()) {
			std::cout << "Couldn't write to " << ofile << std::endl;
			exit(-1);
		}
		while (input)
		{
			std::getline(input, l);
			Position pos(l);
			const Value v = gsl::narrow_cast<Value>(pos.NNScore());
			of << pos.fen(true) << " ce " << v << std::endl;
			++count;
			if (count % 2000 == 0) std::cout << ".";
			if (count % 100000 == 0) std::cout << std::endl;

		}
		of.close();
		input.close();
		return count;
	}

	void testResult() {
		const int64_t begin = now();
		testResult("C:/Users/chrgu_000/Desktop/Data/cutechess/testpositions/stalemate.epd", Result::DRAW);
		testResult("C:/Users/chrgu_000/Desktop/Data/cutechess/testpositions/mate.epd", Result::MATE);
		const int64_t end = now();
		const int64_t runtime = end - begin;
		std::cout << "Runtime: " << runtime / 1000.0 << " s" << std::endl;
	}

	void testCheckQuietCheckMoveGeneration() {
		std::vector<std::string> lines;
		std::ifstream s;
		s.open("C:/Users/chrgu_000/Desktop/Data/cutechess/testpositions/ccrl.epd");
		std::string l;
		if (s.is_open())
		{
			long lineCount = 0;
			while (s)
			{
				std::getline(s, l);
				lines.push_back(l);
				lineCount++;
				if ((lineCount & 8191) == 0) std::cout << ".";
				if (((lineCount & 65535) == 0) || !s) {
					std::cout << lineCount << " Lines read from File - starting to analyze" << std::endl;
					long count = 0;
					for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it) {
						std::string line = *it;
						if (line.length() < 10) continue;
						const size_t indx = line.find(" c0");
						std::string fen = line.substr(0, indx);
						Position p1(fen);
						ValuatedMove* checkGivingMoves = p1.GenerateMoves<QUIET_CHECKS>();
						Position p2(fen);
						ValuatedMove* quietMoves = p2.GenerateMoves<QUIETS>();
						ValuatedMove* cgm = checkGivingMoves;
						ValuatedMove* qm = nullptr;
						Move move;
						//check quiet check move
						while ((move = cgm->move)) {
							Position p3(p1);
							if (p3.ApplyMove(move)) {
								if (!p3.Checked()) {
									std::cout << std::endl << "Move " << toString(move) << " doesn't give check: " << fen << std::endl;
									//__debugbreak();
								}
								bool found = false;
								Move qmove = MOVE_NONE;
								qm = quietMoves;
								while ((qmove = qm->move)) {
									if (qmove == move) {
										found = true;
										break;
									}
									qm++;
								}
								if (!found) {
									std::cout << std::endl << "Move " << toString(move) << " not part of quiet moves: " << fen << std::endl;
									//__debugbreak();
								};
							}
							cgm++;
						}
						//check for completeness
						qm = quietMoves;
						while ((move = qm->move)) {
							Position p3(p1);
							if (p3.ApplyMove(move) && p3.Checked()) {
								cgm = checkGivingMoves;
								Move tmove = MOVE_NONE;
								bool found = false;
								while ((tmove = cgm->move)) {
									if (tmove == move) {
										found = true;
										break;
									}
									cgm++;
								}
								if (!found) {
									std::cout << std::endl << "Move " << toString(move) << " not part of check giving moves: " << fen << std::endl;
									//__debugbreak();
								};
							}
							qm++;
						}
						//if ((count & 1023) == 0)std::cout << std::endl << count << "\t";
						++count;
					}
					lines.clear();
				}
			}
			s.close();
			std::cout << std::endl;
			std::cout << lineCount << " lines read!" << std::endl;
		}
		else
		{
			std::cout << "Unable to open file" << std::endl;
		}
	}

	void testPolyglotKey() {
		uint64_t key = 0x463b96181691fc9c;
		Position pos;
		std::cout << pos.GetHash() << " - " << key << " " << (pos.GetHash() == key) << " " << pos.fen() << std::endl;
		Move move = createMove(E2, E4);
		pos.ApplyMove(move);
		key = 0x823c9b50fd114196;
		std::cout << pos.GetHash() << " - " << key << " " << (pos.GetHash() == key) << " " << pos.fen() << std::endl;
		move = createMove(D7, D5);
		pos.ApplyMove(move);
		key = 0x0756b94461c50fb0;
		std::cout << pos.GetHash() << " - " << key << " " << (pos.GetHash() == key) << " " << pos.fen() << std::endl;
		move = createMove(E4, E5);
		pos.ApplyMove(move);
		key = 0x662fafb965db29d4;
		std::cout << pos.GetHash() << " - " << key << " " << (pos.GetHash() == key) << " " << pos.fen() << std::endl;
		move = createMove(F7, F5);
		pos.ApplyMove(move);
		key = 0x22a48b5a8e47ff78;
		std::cout << pos.GetHash() << " - " << key << " " << (pos.GetHash() == key) << " " << pos.fen() << std::endl;
		move = createMove(E1, E2);
		pos.ApplyMove(move);
		key = 0x652a607ca3f242c1;
		std::cout << pos.GetHash() << " - " << key << " " << (pos.GetHash() == key) << " " << pos.fen() << std::endl;
		move = createMove(E8, F7);
		pos.ApplyMove(move);
		key = 0x00fdd303c946bdd9;
		std::cout << pos.GetHash() << " - " << key << " " << (pos.GetHash() == key) << " " << pos.fen() << std::endl;
	}

	uint64_t perftNodes = 0;
	uint64_t testCount = 0;
	int64_t perftRuntime;
	bool checkPerft(std::string fen, int depth, uint64_t expectedResult, PerftType perftType = BASIC) {
		testCount++;
		Position pos(fen);
		uint64_t perftResult = 0;
		const int64_t begin = now();
		switch (perftType) {
		case BASIC:
			perftResult = perft(pos, depth); break;
		case P1:
			perftResult = perft1(pos, depth); break;
		case P2:
			perftResult = perft2(pos, depth); break;
		case P3:
			perftResult = perft3(pos, depth); break;
		case P4:
			perftResult = perft4(pos, depth); break;
		}
		const int64_t end = now();
		const int64_t runtime = end - begin;
		perftRuntime += runtime;
		perftNodes += expectedResult;
		if (perftResult == expectedResult) {
			if (runtime > 0) {
				std::cout << testCount << "\t" << "OK\t" << depth << "\t" << perftResult << "\t" << runtime << " ms\t" << expectedResult / runtime << " kNodes/s\t" << std::endl << "\t" << fen << std::endl;
			}
			else {
				std::cout << testCount << "\t" << "OK\t" << depth << "\t" << perftResult << "\t" << runtime << " ms\t" << std::endl << "\t" << fen << std::endl;
			}
			return true;
		}
		else {
			perftcomb(pos, depth);
			std::cout << testCount << "\t" << "Error\t" << depth << "\t" << perftResult << "\texpected\t" << expectedResult << "\t" << fen << std::endl;
			return false;
		}
	}

	bool testPerft(PerftType perftType) {
		std::cout << "Starting Perft Suite in Mode " << perftType << std::endl;
		bool result = true;
		testCount = 0;
		perftNodes = 0;
		perftRuntime = 0;
		//if (!perftType) {
		Chess960 = true;
		std::cout << "Chess 960 Positions" << std::endl;
		result = result && checkPerft("rn2k1r1/ppp1pp1p/3p2p1/5bn1/P7/2N2B2/1PPPPP2/2BNK1RR w Gkq - 4 11", 1, 33, perftType);
		result = result && checkPerft("rn2k1r1/ppp1pp1p/3p2p1/5bn1/P7/2N2B2/1PPPPP2/2BNK1RR w Gkq - 4 11", 2, 1072, perftType);
		result = result && checkPerft("rn2k1r1/ppp1pp1p/3p2p1/5bn1/P7/2N2B2/1PPPPP2/2BNK1RR w Gkq - 4 11", 3, 35141, perftType);
		result = result && checkPerft("rn2k1r1/ppp1pp1p/3p2p1/5bn1/P7/2N2B2/1PPPPP2/2BNK1RR w Gkq - 4 11", 4, 1111449, perftType);
		result = result && checkPerft("rn2k1r1/ppp1pp1p/3p2p1/5bn1/P7/2N2B2/1PPPPP2/2BNK1RR w Gkq - 4 11", 5, 37095094, perftType);
		result = result && checkPerft("nrbkqbnr/pppppppp/8/8/8/8/PPPPPPPP/NRBKQBNR w KQkq - 0 1", 1, 19, perftType);
		result = result && checkPerft("nrbkqbnr/pppppppp/8/8/8/8/PPPPPPPP/NRBKQBNR w KQkq - 0 1", 2, 361, perftType);
		result = result && checkPerft("nrbkqbnr/pppppppp/8/8/8/8/PPPPPPPP/NRBKQBNR w KQkq - 0 1", 3, 7735, perftType);
		result = result && checkPerft("nrbkqbnr/pppppppp/8/8/8/8/PPPPPPPP/NRBKQBNR w KQkq - 0 1", 4, 164966, perftType);
		result = result && checkPerft("nrbkqbnr/pppppppp/8/8/8/8/PPPPPPPP/NRBKQBNR w KQkq - 0 1", 5, 3962549, perftType);
		result = result && checkPerft("nrbkqbnr/pppppppp/8/8/8/8/PPPPPPPP/NRBKQBNR w KQkq - 0 1", 6, 94328606, perftType);
		result = result && checkPerft("2rkr3/5PP1/8/5Q2/5q2/8/5pp1/2RKR3 w KQkq - 0 1", 5, 94370149, perftType);
		result = result && checkPerft("rqkrbnnb/pppppppp/8/8/8/8/PPPPPPPP/RQKRBNNB w KQkq - 0 1", 6, 111825069, perftType);
		//}
		Chess960 = false;
		std::cout << "Initial Position" << std::endl;
		result = result && checkPerft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 1, 20, perftType);
		result = result && checkPerft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 2, 400, perftType);
		result = result && checkPerft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 3, 8902, perftType);
		result = result && checkPerft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 4, 197281, perftType);
		result = result && checkPerft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 5, 4865609, perftType);
		result = result && checkPerft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 6, 119060324, perftType);
		std::cout << "Kiwi Pete" << std::endl;
		result = result && checkPerft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 1, 48, perftType);
		result = result && checkPerft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 2, 2039, perftType);
		result = result && checkPerft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 3, 97862, perftType);
		result = result && checkPerft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 4, 4085603, perftType);
		result = result && checkPerft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 5, 193690690, perftType);
		std::cout << "Standard Chess positions" << std::endl;
		result = result && checkPerft("4k3/8/8/8/8/8/8/4K2R w K - 0 1", 1, 15, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/4K2R w K - 0 1", 2, 66, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/4K2R w K - 0 1", 3, 1197, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/4K2R w K - 0 1", 4, 7059, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/4K2R w K - 0 1", 5, 133987, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/4K2R w K - 0 1", 6, 764643, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K3 w Q - 0 1", 1, 16, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K3 w Q - 0 1", 2, 71, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K3 w Q - 0 1", 3, 1287, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K3 w Q - 0 1", 4, 7626, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K3 w Q - 0 1", 5, 145232, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K3 w Q - 0 1", 6, 846648, perftType);
		result = result && checkPerft("4k2r/8/8/8/8/8/8/4K3 w k - 0 1", 1, 5, perftType);
		result = result && checkPerft("4k2r/8/8/8/8/8/8/4K3 w k - 0 1", 2, 75, perftType);
		result = result && checkPerft("4k2r/8/8/8/8/8/8/4K3 w k - 0 1", 3, 459, perftType);
		result = result && checkPerft("4k2r/8/8/8/8/8/8/4K3 w k - 0 1", 4, 8290, perftType);
		result = result && checkPerft("4k2r/8/8/8/8/8/8/4K3 w k - 0 1", 5, 47635, perftType);
		result = result && checkPerft("4k2r/8/8/8/8/8/8/4K3 w k - 0 1", 6, 899442, perftType);
		result = result && checkPerft("r3k3/8/8/8/8/8/8/4K3 w q - 0 1", 1, 5, perftType);
		result = result && checkPerft("r3k3/8/8/8/8/8/8/4K3 w q - 0 1", 2, 80, perftType);
		result = result && checkPerft("r3k3/8/8/8/8/8/8/4K3 w q - 0 1", 3, 493, perftType);
		result = result && checkPerft("r3k3/8/8/8/8/8/8/4K3 w q - 0 1", 4, 8897, perftType);
		result = result && checkPerft("r3k3/8/8/8/8/8/8/4K3 w q - 0 1", 5, 52710, perftType);
		result = result && checkPerft("r3k3/8/8/8/8/8/8/4K3 w q - 0 1", 6, 1001523, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1", 1, 26, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1", 2, 112, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1", 3, 3189, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1", 4, 17945, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1", 5, 532933, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1", 6, 2788982, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/4K3 w kq - 0 1", 1, 5, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/4K3 w kq - 0 1", 2, 130, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/4K3 w kq - 0 1", 3, 782, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/4K3 w kq - 0 1", 4, 22180, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/4K3 w kq - 0 1", 5, 118882, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/4K3 w kq - 0 1", 6, 3517770, perftType);
		result = result && checkPerft("8/8/8/8/8/8/6k1/4K2R w K - 0 1", 1, 12, perftType);
		result = result && checkPerft("8/8/8/8/8/8/6k1/4K2R w K - 0 1", 2, 38, perftType);
		result = result && checkPerft("8/8/8/8/8/8/6k1/4K2R w K - 0 1", 3, 564, perftType);
		result = result && checkPerft("8/8/8/8/8/8/6k1/4K2R w K - 0 1", 4, 2219, perftType);
		result = result && checkPerft("8/8/8/8/8/8/6k1/4K2R w K - 0 1", 5, 37735, perftType);
		result = result && checkPerft("8/8/8/8/8/8/6k1/4K2R w K - 0 1", 6, 185867, perftType);
		result = result && checkPerft("8/8/8/8/8/8/1k6/R3K3 w Q - 0 1", 1, 15, perftType);
		result = result && checkPerft("8/8/8/8/8/8/1k6/R3K3 w Q - 0 1", 2, 65, perftType);
		result = result && checkPerft("8/8/8/8/8/8/1k6/R3K3 w Q - 0 1", 3, 1018, perftType);
		result = result && checkPerft("8/8/8/8/8/8/1k6/R3K3 w Q - 0 1", 4, 4573, perftType);
		result = result && checkPerft("8/8/8/8/8/8/1k6/R3K3 w Q - 0 1", 5, 80619, perftType);
		result = result && checkPerft("8/8/8/8/8/8/1k6/R3K3 w Q - 0 1", 6, 413018, perftType);
		result = result && checkPerft("4k2r/6K1/8/8/8/8/8/8 w k - 0 1", 1, 3, perftType);
		result = result && checkPerft("4k2r/6K1/8/8/8/8/8/8 w k - 0 1", 2, 32, perftType);
		result = result && checkPerft("4k2r/6K1/8/8/8/8/8/8 w k - 0 1", 3, 134, perftType);
		result = result && checkPerft("4k2r/6K1/8/8/8/8/8/8 w k - 0 1", 4, 2073, perftType);
		result = result && checkPerft("4k2r/6K1/8/8/8/8/8/8 w k - 0 1", 5, 10485, perftType);
		result = result && checkPerft("4k2r/6K1/8/8/8/8/8/8 w k - 0 1", 6, 179869, perftType);
		result = result && checkPerft("r3k3/1K6/8/8/8/8/8/8 w q - 0 1", 1, 4, perftType);
		result = result && checkPerft("r3k3/1K6/8/8/8/8/8/8 w q - 0 1", 2, 49, perftType);
		result = result && checkPerft("r3k3/1K6/8/8/8/8/8/8 w q - 0 1", 3, 243, perftType);
		result = result && checkPerft("r3k3/1K6/8/8/8/8/8/8 w q - 0 1", 4, 3991, perftType);
		result = result && checkPerft("r3k3/1K6/8/8/8/8/8/8 w q - 0 1", 5, 20780, perftType);
		result = result && checkPerft("r3k3/1K6/8/8/8/8/8/8 w q - 0 1", 6, 367724, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", 1, 26, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", 2, 568, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", 3, 13744, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", 4, 314346, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", 5, 7594526, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", 6, 179862938, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/1R2K2R w Kkq - 0 1", 1, 25, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/1R2K2R w Kkq - 0 1", 2, 567, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/1R2K2R w Kkq - 0 1", 3, 14095, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/1R2K2R w Kkq - 0 1", 4, 328965, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/1R2K2R w Kkq - 0 1", 5, 8153719, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/1R2K2R w Kkq - 0 1", 6, 195629489, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/2R1K2R w Kkq - 0 1", 1, 25, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/2R1K2R w Kkq - 0 1", 2, 548, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/2R1K2R w Kkq - 0 1", 3, 13502, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/2R1K2R w Kkq - 0 1", 4, 312835, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/2R1K2R w Kkq - 0 1", 5, 7736373, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/2R1K2R w Kkq - 0 1", 6, 184411439, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K1R1 w Qkq - 0 1", 1, 25, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K1R1 w Qkq - 0 1", 2, 547, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K1R1 w Qkq - 0 1", 3, 13579, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K1R1 w Qkq - 0 1", 4, 316214, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K1R1 w Qkq - 0 1", 5, 7878456, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K1R1 w Qkq - 0 1", 6, 189224276, perftType);
		result = result && checkPerft("1r2k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1", 1, 26, perftType);
		result = result && checkPerft("1r2k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1", 2, 583, perftType);
		result = result && checkPerft("1r2k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1", 3, 14252, perftType);
		result = result && checkPerft("1r2k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1", 4, 334705, perftType);
		result = result && checkPerft("1r2k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1", 5, 8198901, perftType);
		result = result && checkPerft("1r2k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1", 6, 198328929, perftType);
		result = result && checkPerft("2r1k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1", 1, 25, perftType);
		result = result && checkPerft("2r1k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1", 2, 560, perftType);
		result = result && checkPerft("2r1k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1", 3, 13592, perftType);
		result = result && checkPerft("2r1k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1", 4, 317324, perftType);
		result = result && checkPerft("2r1k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1", 5, 7710115, perftType);
		result = result && checkPerft("2r1k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1", 6, 185959088, perftType);
		result = result && checkPerft("r3k1r1/8/8/8/8/8/8/R3K2R w KQq - 0 1", 1, 25, perftType);
		result = result && checkPerft("r3k1r1/8/8/8/8/8/8/R3K2R w KQq - 0 1", 2, 560, perftType);
		result = result && checkPerft("r3k1r1/8/8/8/8/8/8/R3K2R w KQq - 0 1", 3, 13607, perftType);
		result = result && checkPerft("r3k1r1/8/8/8/8/8/8/R3K2R w KQq - 0 1", 4, 320792, perftType);
		result = result && checkPerft("r3k1r1/8/8/8/8/8/8/R3K2R w KQq - 0 1", 5, 7848606, perftType);
		result = result && checkPerft("r3k1r1/8/8/8/8/8/8/R3K2R w KQq - 0 1", 6, 190755813, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/4K2R b K - 0 1", 1, 5, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/4K2R b K - 0 1", 2, 75, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/4K2R b K - 0 1", 3, 459, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/4K2R b K - 0 1", 4, 8290, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/4K2R b K - 0 1", 5, 47635, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/4K2R b K - 0 1", 6, 899442, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K3 b Q - 0 1", 1, 5, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K3 b Q - 0 1", 2, 80, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K3 b Q - 0 1", 3, 493, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K3 b Q - 0 1", 4, 8897, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K3 b Q - 0 1", 5, 52710, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K3 b Q - 0 1", 6, 1001523, perftType);
		result = result && checkPerft("4k2r/8/8/8/8/8/8/4K3 b k - 0 1", 1, 15, perftType);
		result = result && checkPerft("4k2r/8/8/8/8/8/8/4K3 b k - 0 1", 2, 66, perftType);
		result = result && checkPerft("4k2r/8/8/8/8/8/8/4K3 b k - 0 1", 3, 1197, perftType);
		result = result && checkPerft("4k2r/8/8/8/8/8/8/4K3 b k - 0 1", 4, 7059, perftType);
		result = result && checkPerft("4k2r/8/8/8/8/8/8/4K3 b k - 0 1", 5, 133987, perftType);
		result = result && checkPerft("4k2r/8/8/8/8/8/8/4K3 b k - 0 1", 6, 764643, perftType);
		result = result && checkPerft("r3k3/8/8/8/8/8/8/4K3 b q - 0 1", 1, 16, perftType);
		result = result && checkPerft("r3k3/8/8/8/8/8/8/4K3 b q - 0 1", 2, 71, perftType);
		result = result && checkPerft("r3k3/8/8/8/8/8/8/4K3 b q - 0 1", 3, 1287, perftType);
		result = result && checkPerft("r3k3/8/8/8/8/8/8/4K3 b q - 0 1", 4, 7626, perftType);
		result = result && checkPerft("r3k3/8/8/8/8/8/8/4K3 b q - 0 1", 5, 145232, perftType);
		result = result && checkPerft("r3k3/8/8/8/8/8/8/4K3 b q - 0 1", 6, 846648, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K2R b KQ - 0 1", 1, 5, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K2R b KQ - 0 1", 2, 130, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K2R b KQ - 0 1", 3, 782, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K2R b KQ - 0 1", 4, 22180, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K2R b KQ - 0 1", 5, 118882, perftType);
		result = result && checkPerft("4k3/8/8/8/8/8/8/R3K2R b KQ - 0 1", 6, 3517770, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1", 1, 26, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1", 2, 112, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1", 3, 3189, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1", 4, 17945, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1", 5, 532933, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1", 6, 2788982, perftType);
		result = result && checkPerft("8/8/8/8/8/8/6k1/4K2R b K - 0 1", 1, 3, perftType);
		result = result && checkPerft("8/8/8/8/8/8/6k1/4K2R b K - 0 1", 2, 32, perftType);
		result = result && checkPerft("8/8/8/8/8/8/6k1/4K2R b K - 0 1", 3, 134, perftType);
		result = result && checkPerft("8/8/8/8/8/8/6k1/4K2R b K - 0 1", 4, 2073, perftType);
		result = result && checkPerft("8/8/8/8/8/8/6k1/4K2R b K - 0 1", 5, 10485, perftType);
		result = result && checkPerft("8/8/8/8/8/8/6k1/4K2R b K - 0 1", 6, 179869, perftType);
		result = result && checkPerft("8/8/8/8/8/8/1k6/R3K3 b Q - 0 1", 1, 4, perftType);
		result = result && checkPerft("8/8/8/8/8/8/1k6/R3K3 b Q - 0 1", 2, 49, perftType);
		result = result && checkPerft("8/8/8/8/8/8/1k6/R3K3 b Q - 0 1", 3, 243, perftType);
		result = result && checkPerft("8/8/8/8/8/8/1k6/R3K3 b Q - 0 1", 4, 3991, perftType);
		result = result && checkPerft("8/8/8/8/8/8/1k6/R3K3 b Q - 0 1", 5, 20780, perftType);
		result = result && checkPerft("8/8/8/8/8/8/1k6/R3K3 b Q - 0 1", 6, 367724, perftType);
		result = result && checkPerft("4k2r/6K1/8/8/8/8/8/8 b k - 0 1", 1, 12, perftType);
		result = result && checkPerft("4k2r/6K1/8/8/8/8/8/8 b k - 0 1", 2, 38, perftType);
		result = result && checkPerft("4k2r/6K1/8/8/8/8/8/8 b k - 0 1", 3, 564, perftType);
		result = result && checkPerft("4k2r/6K1/8/8/8/8/8/8 b k - 0 1", 4, 2219, perftType);
		result = result && checkPerft("4k2r/6K1/8/8/8/8/8/8 b k - 0 1", 5, 37735, perftType);
		result = result && checkPerft("4k2r/6K1/8/8/8/8/8/8 b k - 0 1", 6, 185867, perftType);
		result = result && checkPerft("r3k3/1K6/8/8/8/8/8/8 b q - 0 1", 1, 15, perftType);
		result = result && checkPerft("r3k3/1K6/8/8/8/8/8/8 b q - 0 1", 2, 65, perftType);
		result = result && checkPerft("r3k3/1K6/8/8/8/8/8/8 b q - 0 1", 3, 1018, perftType);
		result = result && checkPerft("r3k3/1K6/8/8/8/8/8/8 b q - 0 1", 4, 4573, perftType);
		result = result && checkPerft("r3k3/1K6/8/8/8/8/8/8 b q - 0 1", 5, 80619, perftType);
		result = result && checkPerft("r3k3/1K6/8/8/8/8/8/8 b q - 0 1", 6, 413018, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1", 1, 26, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1", 2, 568, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1", 3, 13744, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1", 4, 314346, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1", 5, 7594526, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1", 6, 179862938, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/1R2K2R b Kkq - 0 1", 1, 26, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/1R2K2R b Kkq - 0 1", 2, 583, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/1R2K2R b Kkq - 0 1", 3, 14252, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/1R2K2R b Kkq - 0 1", 4, 334705, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/1R2K2R b Kkq - 0 1", 5, 8198901, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/1R2K2R b Kkq - 0 1", 6, 198328929, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/2R1K2R b Kkq - 0 1", 1, 25, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/2R1K2R b Kkq - 0 1", 2, 560, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/2R1K2R b Kkq - 0 1", 3, 13592, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/2R1K2R b Kkq - 0 1", 4, 317324, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/2R1K2R b Kkq - 0 1", 5, 7710115, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/2R1K2R b Kkq - 0 1", 6, 185959088, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K1R1 b Qkq - 0 1", 1, 25, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K1R1 b Qkq - 0 1", 2, 560, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K1R1 b Qkq - 0 1", 3, 13607, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K1R1 b Qkq - 0 1", 4, 320792, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K1R1 b Qkq - 0 1", 5, 7848606, perftType);
		result = result && checkPerft("r3k2r/8/8/8/8/8/8/R3K1R1 b Qkq - 0 1", 6, 190755813, perftType);
		result = result && checkPerft("1r2k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1", 1, 25, perftType);
		result = result && checkPerft("1r2k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1", 2, 567, perftType);
		result = result && checkPerft("1r2k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1", 3, 14095, perftType);
		result = result && checkPerft("1r2k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1", 4, 328965, perftType);
		result = result && checkPerft("1r2k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1", 5, 8153719, perftType);
		result = result && checkPerft("1r2k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1", 6, 195629489, perftType);
		result = result && checkPerft("2r1k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1", 1, 25, perftType);
		result = result && checkPerft("2r1k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1", 2, 548, perftType);
		result = result && checkPerft("2r1k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1", 3, 13502, perftType);
		result = result && checkPerft("2r1k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1", 4, 312835, perftType);
		result = result && checkPerft("2r1k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1", 5, 7736373, perftType);
		result = result && checkPerft("2r1k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1", 6, 184411439, perftType);
		result = result && checkPerft("r3k1r1/8/8/8/8/8/8/R3K2R b KQq - 0 1", 1, 25, perftType);
		result = result && checkPerft("r3k1r1/8/8/8/8/8/8/R3K2R b KQq - 0 1", 2, 547, perftType);
		result = result && checkPerft("r3k1r1/8/8/8/8/8/8/R3K2R b KQq - 0 1", 3, 13579, perftType);
		result = result && checkPerft("r3k1r1/8/8/8/8/8/8/R3K2R b KQq - 0 1", 4, 316214, perftType);
		result = result && checkPerft("r3k1r1/8/8/8/8/8/8/R3K2R b KQq - 0 1", 5, 7878456, perftType);
		result = result && checkPerft("r3k1r1/8/8/8/8/8/8/R3K2R b KQq - 0 1", 6, 189224276, perftType);
		result = result && checkPerft("8/1n4N1/2k5/8/8/5K2/1N4n1/8 w - - 0 1", 1, 14, perftType);
		result = result && checkPerft("8/1n4N1/2k5/8/8/5K2/1N4n1/8 w - - 0 1", 2, 195, perftType);
		result = result && checkPerft("8/1n4N1/2k5/8/8/5K2/1N4n1/8 w - - 0 1", 3, 2760, perftType);
		result = result && checkPerft("8/1n4N1/2k5/8/8/5K2/1N4n1/8 w - - 0 1", 4, 38675, perftType);
		result = result && checkPerft("8/1n4N1/2k5/8/8/5K2/1N4n1/8 w - - 0 1", 5, 570726, perftType);
		result = result && checkPerft("8/1n4N1/2k5/8/8/5K2/1N4n1/8 w - - 0 1", 6, 8107539, perftType);
		result = result && checkPerft("8/1k6/8/5N2/8/4n3/8/2K5 w - - 0 1", 1, 11, perftType);
		result = result && checkPerft("8/1k6/8/5N2/8/4n3/8/2K5 w - - 0 1", 2, 156, perftType);
		result = result && checkPerft("8/1k6/8/5N2/8/4n3/8/2K5 w - - 0 1", 3, 1636, perftType);
		result = result && checkPerft("8/1k6/8/5N2/8/4n3/8/2K5 w - - 0 1", 4, 20534, perftType);
		result = result && checkPerft("8/1k6/8/5N2/8/4n3/8/2K5 w - - 0 1", 5, 223507, perftType);
		result = result && checkPerft("8/1k6/8/5N2/8/4n3/8/2K5 w - - 0 1", 6, 2594412, perftType);
		result = result && checkPerft("8/8/4k3/3Nn3/3nN3/4K3/8/8 w - - 0 1", 1, 19, perftType);
		result = result && checkPerft("8/8/4k3/3Nn3/3nN3/4K3/8/8 w - - 0 1", 2, 289, perftType);
		result = result && checkPerft("8/8/4k3/3Nn3/3nN3/4K3/8/8 w - - 0 1", 3, 4442, perftType);
		result = result && checkPerft("8/8/4k3/3Nn3/3nN3/4K3/8/8 w - - 0 1", 4, 73584, perftType);
		result = result && checkPerft("8/8/4k3/3Nn3/3nN3/4K3/8/8 w - - 0 1", 5, 1198299, perftType);
		result = result && checkPerft("8/8/4k3/3Nn3/3nN3/4K3/8/8 w - - 0 1", 6, 19870403, perftType);
		result = result && checkPerft("K7/8/2n5/1n6/8/8/8/k6N w - - 0 1", 1, 3, perftType);
		result = result && checkPerft("K7/8/2n5/1n6/8/8/8/k6N w - - 0 1", 2, 51, perftType);
		result = result && checkPerft("K7/8/2n5/1n6/8/8/8/k6N w - - 0 1", 3, 345, perftType);
		result = result && checkPerft("K7/8/2n5/1n6/8/8/8/k6N w - - 0 1", 4, 5301, perftType);
		result = result && checkPerft("K7/8/2n5/1n6/8/8/8/k6N w - - 0 1", 5, 38348, perftType);
		result = result && checkPerft("K7/8/2n5/1n6/8/8/8/k6N w - - 0 1", 6, 588695, perftType);
		result = result && checkPerft("k7/8/2N5/1N6/8/8/8/K6n w - - 0 1", 1, 17, perftType);
		result = result && checkPerft("k7/8/2N5/1N6/8/8/8/K6n w - - 0 1", 2, 54, perftType);
		result = result && checkPerft("k7/8/2N5/1N6/8/8/8/K6n w - - 0 1", 3, 835, perftType);
		result = result && checkPerft("k7/8/2N5/1N6/8/8/8/K6n w - - 0 1", 4, 5910, perftType);
		result = result && checkPerft("k7/8/2N5/1N6/8/8/8/K6n w - - 0 1", 5, 92250, perftType);
		result = result && checkPerft("k7/8/2N5/1N6/8/8/8/K6n w - - 0 1", 6, 688780, perftType);
		result = result && checkPerft("8/1n4N1/2k5/8/8/5K2/1N4n1/8 b - - 0 1", 1, 15, perftType);
		result = result && checkPerft("8/1n4N1/2k5/8/8/5K2/1N4n1/8 b - - 0 1", 2, 193, perftType);
		result = result && checkPerft("8/1n4N1/2k5/8/8/5K2/1N4n1/8 b - - 0 1", 3, 2816, perftType);
		result = result && checkPerft("8/1n4N1/2k5/8/8/5K2/1N4n1/8 b - - 0 1", 4, 40039, perftType);
		result = result && checkPerft("8/1n4N1/2k5/8/8/5K2/1N4n1/8 b - - 0 1", 5, 582642, perftType);
		result = result && checkPerft("8/1n4N1/2k5/8/8/5K2/1N4n1/8 b - - 0 1", 6, 8503277, perftType);
		result = result && checkPerft("8/1k6/8/5N2/8/4n3/8/2K5 b - - 0 1", 1, 16, perftType);
		result = result && checkPerft("8/1k6/8/5N2/8/4n3/8/2K5 b - - 0 1", 2, 180, perftType);
		result = result && checkPerft("8/1k6/8/5N2/8/4n3/8/2K5 b - - 0 1", 3, 2290, perftType);
		result = result && checkPerft("8/1k6/8/5N2/8/4n3/8/2K5 b - - 0 1", 4, 24640, perftType);
		result = result && checkPerft("8/1k6/8/5N2/8/4n3/8/2K5 b - - 0 1", 5, 288141, perftType);
		result = result && checkPerft("8/1k6/8/5N2/8/4n3/8/2K5 b - - 0 1", 6, 3147566, perftType);
		result = result && checkPerft("8/8/3K4/3Nn3/3nN3/4k3/8/8 b - - 0 1", 1, 4, perftType);
		result = result && checkPerft("8/8/3K4/3Nn3/3nN3/4k3/8/8 b - - 0 1", 2, 68, perftType);
		result = result && checkPerft("8/8/3K4/3Nn3/3nN3/4k3/8/8 b - - 0 1", 3, 1118, perftType);
		result = result && checkPerft("8/8/3K4/3Nn3/3nN3/4k3/8/8 b - - 0 1", 4, 16199, perftType);
		result = result && checkPerft("8/8/3K4/3Nn3/3nN3/4k3/8/8 b - - 0 1", 5, 281190, perftType);
		result = result && checkPerft("8/8/3K4/3Nn3/3nN3/4k3/8/8 b - - 0 1", 6, 4405103, perftType);
		result = result && checkPerft("K7/8/2n5/1n6/8/8/8/k6N b - - 0 1", 1, 17, perftType);
		result = result && checkPerft("K7/8/2n5/1n6/8/8/8/k6N b - - 0 1", 2, 54, perftType);
		result = result && checkPerft("K7/8/2n5/1n6/8/8/8/k6N b - - 0 1", 3, 835, perftType);
		result = result && checkPerft("K7/8/2n5/1n6/8/8/8/k6N b - - 0 1", 4, 5910, perftType);
		result = result && checkPerft("K7/8/2n5/1n6/8/8/8/k6N b - - 0 1", 5, 92250, perftType);
		result = result && checkPerft("K7/8/2n5/1n6/8/8/8/k6N b - - 0 1", 6, 688780, perftType);
		result = result && checkPerft("k7/8/2N5/1N6/8/8/8/K6n b - - 0 1", 1, 3, perftType);
		result = result && checkPerft("k7/8/2N5/1N6/8/8/8/K6n b - - 0 1", 2, 51, perftType);
		result = result && checkPerft("k7/8/2N5/1N6/8/8/8/K6n b - - 0 1", 3, 345, perftType);
		result = result && checkPerft("k7/8/2N5/1N6/8/8/8/K6n b - - 0 1", 4, 5301, perftType);
		result = result && checkPerft("k7/8/2N5/1N6/8/8/8/K6n b - - 0 1", 5, 38348, perftType);
		result = result && checkPerft("k7/8/2N5/1N6/8/8/8/K6n b - - 0 1", 6, 588695, perftType);
		result = result && checkPerft("B6b/8/8/8/2K5/4k3/8/b6B w - - 0 1", 1, 17, perftType);
		result = result && checkPerft("B6b/8/8/8/2K5/4k3/8/b6B w - - 0 1", 2, 278, perftType);
		result = result && checkPerft("B6b/8/8/8/2K5/4k3/8/b6B w - - 0 1", 3, 4607, perftType);
		result = result && checkPerft("B6b/8/8/8/2K5/4k3/8/b6B w - - 0 1", 4, 76778, perftType);
		result = result && checkPerft("B6b/8/8/8/2K5/4k3/8/b6B w - - 0 1", 5, 1320507, perftType);
		result = result && checkPerft("B6b/8/8/8/2K5/4k3/8/b6B w - - 0 1", 6, 22823890, perftType);
		result = result && checkPerft("8/8/1B6/7b/7k/8/2B1b3/7K w - - 0 1", 1, 21, perftType);
		result = result && checkPerft("8/8/1B6/7b/7k/8/2B1b3/7K w - - 0 1", 2, 316, perftType);
		result = result && checkPerft("8/8/1B6/7b/7k/8/2B1b3/7K w - - 0 1", 3, 5744, perftType);
		result = result && checkPerft("8/8/1B6/7b/7k/8/2B1b3/7K w - - 0 1", 4, 93338, perftType);
		result = result && checkPerft("8/8/1B6/7b/7k/8/2B1b3/7K w - - 0 1", 5, 1713368, perftType);
		result = result && checkPerft("8/8/1B6/7b/7k/8/2B1b3/7K w - - 0 1", 6, 28861171, perftType);
		result = result && checkPerft("k7/B7/1B6/1B6/8/8/8/K6b w - - 0 1", 1, 21, perftType);
		result = result && checkPerft("k7/B7/1B6/1B6/8/8/8/K6b w - - 0 1", 2, 144, perftType);
		result = result && checkPerft("k7/B7/1B6/1B6/8/8/8/K6b w - - 0 1", 3, 3242, perftType);
		result = result && checkPerft("k7/B7/1B6/1B6/8/8/8/K6b w - - 0 1", 4, 32955, perftType);
		result = result && checkPerft("k7/B7/1B6/1B6/8/8/8/K6b w - - 0 1", 5, 787524, perftType);
		result = result && checkPerft("k7/B7/1B6/1B6/8/8/8/K6b w - - 0 1", 6, 7881673, perftType);
		result = result && checkPerft("K7/b7/1b6/1b6/8/8/8/k6B w - - 0 1", 1, 7, perftType);
		result = result && checkPerft("K7/b7/1b6/1b6/8/8/8/k6B w - - 0 1", 2, 143, perftType);
		result = result && checkPerft("K7/b7/1b6/1b6/8/8/8/k6B w - - 0 1", 3, 1416, perftType);
		result = result && checkPerft("K7/b7/1b6/1b6/8/8/8/k6B w - - 0 1", 4, 31787, perftType);
		result = result && checkPerft("K7/b7/1b6/1b6/8/8/8/k6B w - - 0 1", 5, 310862, perftType);
		result = result && checkPerft("K7/b7/1b6/1b6/8/8/8/k6B w - - 0 1", 6, 7382896, perftType);
		result = result && checkPerft("B6b/8/8/8/2K5/5k2/8/b6B b - - 0 1", 1, 6, perftType);
		result = result && checkPerft("B6b/8/8/8/2K5/5k2/8/b6B b - - 0 1", 2, 106, perftType);
		result = result && checkPerft("B6b/8/8/8/2K5/5k2/8/b6B b - - 0 1", 3, 1829, perftType);
		result = result && checkPerft("B6b/8/8/8/2K5/5k2/8/b6B b - - 0 1", 4, 31151, perftType);
		result = result && checkPerft("B6b/8/8/8/2K5/5k2/8/b6B b - - 0 1", 5, 530585, perftType);
		result = result && checkPerft("B6b/8/8/8/2K5/5k2/8/b6B b - - 0 1", 6, 9250746, perftType);
		result = result && checkPerft("8/8/1B6/7b/7k/8/2B1b3/7K b - - 0 1", 1, 17, perftType);
		result = result && checkPerft("8/8/1B6/7b/7k/8/2B1b3/7K b - - 0 1", 2, 309, perftType);
		result = result && checkPerft("8/8/1B6/7b/7k/8/2B1b3/7K b - - 0 1", 3, 5133, perftType);
		result = result && checkPerft("8/8/1B6/7b/7k/8/2B1b3/7K b - - 0 1", 4, 93603, perftType);
		result = result && checkPerft("8/8/1B6/7b/7k/8/2B1b3/7K b - - 0 1", 5, 1591064, perftType);
		result = result && checkPerft("8/8/1B6/7b/7k/8/2B1b3/7K b - - 0 1", 6, 29027891, perftType);
		result = result && checkPerft("k7/B7/1B6/1B6/8/8/8/K6b b - - 0 1", 1, 7, perftType);
		result = result && checkPerft("k7/B7/1B6/1B6/8/8/8/K6b b - - 0 1", 2, 143, perftType);
		result = result && checkPerft("k7/B7/1B6/1B6/8/8/8/K6b b - - 0 1", 3, 1416, perftType);
		result = result && checkPerft("k7/B7/1B6/1B6/8/8/8/K6b b - - 0 1", 4, 31787, perftType);
		result = result && checkPerft("k7/B7/1B6/1B6/8/8/8/K6b b - - 0 1", 5, 310862, perftType);
		result = result && checkPerft("k7/B7/1B6/1B6/8/8/8/K6b b - - 0 1", 6, 7382896, perftType);
		result = result && checkPerft("K7/b7/1b6/1b6/8/8/8/k6B b - - 0 1", 1, 21, perftType);
		result = result && checkPerft("K7/b7/1b6/1b6/8/8/8/k6B b - - 0 1", 2, 144, perftType);
		result = result && checkPerft("K7/b7/1b6/1b6/8/8/8/k6B b - - 0 1", 3, 3242, perftType);
		result = result && checkPerft("K7/b7/1b6/1b6/8/8/8/k6B b - - 0 1", 4, 32955, perftType);
		result = result && checkPerft("K7/b7/1b6/1b6/8/8/8/k6B b - - 0 1", 5, 787524, perftType);
		result = result && checkPerft("K7/b7/1b6/1b6/8/8/8/k6B b - - 0 1", 6, 7881673, perftType);
		result = result && checkPerft("7k/RR6/8/8/8/8/rr6/7K w - - 0 1", 1, 19, perftType);
		result = result && checkPerft("7k/RR6/8/8/8/8/rr6/7K w - - 0 1", 2, 275, perftType);
		result = result && checkPerft("7k/RR6/8/8/8/8/rr6/7K w - - 0 1", 3, 5300, perftType);
		result = result && checkPerft("7k/RR6/8/8/8/8/rr6/7K w - - 0 1", 4, 104342, perftType);
		result = result && checkPerft("7k/RR6/8/8/8/8/rr6/7K w - - 0 1", 5, 2161211, perftType);
		result = result && checkPerft("7k/RR6/8/8/8/8/rr6/7K w - - 0 1", 6, 44956585, perftType);
		result = result && checkPerft("R6r/8/8/2K5/5k2/8/8/r6R w - - 0 1", 1, 36, perftType);
		result = result && checkPerft("R6r/8/8/2K5/5k2/8/8/r6R w - - 0 1", 2, 1027, perftType);
		result = result && checkPerft("R6r/8/8/2K5/5k2/8/8/r6R w - - 0 1", 3, 29215, perftType);
		result = result && checkPerft("R6r/8/8/2K5/5k2/8/8/r6R w - - 0 1", 4, 771461, perftType);
		result = result && checkPerft("R6r/8/8/2K5/5k2/8/8/r6R w - - 0 1", 5, 20506480, perftType);
		result = result && checkPerft("R6r/8/8/2K5/5k2/8/8/r6R w - - 0 1", 6, 525169084, perftType);
		result = result && checkPerft("7k/RR6/8/8/8/8/rr6/7K b - - 0 1", 1, 19, perftType);
		result = result && checkPerft("7k/RR6/8/8/8/8/rr6/7K b - - 0 1", 2, 275, perftType);
		result = result && checkPerft("7k/RR6/8/8/8/8/rr6/7K b - - 0 1", 3, 5300, perftType);
		result = result && checkPerft("7k/RR6/8/8/8/8/rr6/7K b - - 0 1", 4, 104342, perftType);
		result = result && checkPerft("7k/RR6/8/8/8/8/rr6/7K b - - 0 1", 5, 2161211, perftType);
		result = result && checkPerft("7k/RR6/8/8/8/8/rr6/7K b - - 0 1", 6, 44956585, perftType);
		result = result && checkPerft("R6r/8/8/2K5/5k2/8/8/r6R b - - 0 1", 1, 36, perftType);
		result = result && checkPerft("R6r/8/8/2K5/5k2/8/8/r6R b - - 0 1", 2, 1027, perftType);
		result = result && checkPerft("R6r/8/8/2K5/5k2/8/8/r6R b - - 0 1", 3, 29227, perftType);
		result = result && checkPerft("R6r/8/8/2K5/5k2/8/8/r6R b - - 0 1", 4, 771368, perftType);
		result = result && checkPerft("R6r/8/8/2K5/5k2/8/8/r6R b - - 0 1", 5, 20521342, perftType);
		result = result && checkPerft("R6r/8/8/2K5/5k2/8/8/r6R b - - 0 1", 6, 524966748, perftType);
		result = result && checkPerft("6kq/8/8/8/8/8/8/7K w - - 0 1", 1, 2, perftType);
		result = result && checkPerft("6kq/8/8/8/8/8/8/7K w - - 0 1", 2, 36, perftType);
		result = result && checkPerft("6kq/8/8/8/8/8/8/7K w - - 0 1", 3, 143, perftType);
		result = result && checkPerft("6kq/8/8/8/8/8/8/7K w - - 0 1", 4, 3637, perftType);
		result = result && checkPerft("6kq/8/8/8/8/8/8/7K w - - 0 1", 5, 14893, perftType);
		result = result && checkPerft("6kq/8/8/8/8/8/8/7K w - - 0 1", 6, 391507, perftType);
		result = result && checkPerft("6KQ/8/8/8/8/8/8/7k b - - 0 1", 1, 2, perftType);
		result = result && checkPerft("6KQ/8/8/8/8/8/8/7k b - - 0 1", 2, 36, perftType);
		result = result && checkPerft("6KQ/8/8/8/8/8/8/7k b - - 0 1", 3, 143, perftType);
		result = result && checkPerft("6KQ/8/8/8/8/8/8/7k b - - 0 1", 4, 3637, perftType);
		result = result && checkPerft("6KQ/8/8/8/8/8/8/7k b - - 0 1", 5, 14893, perftType);
		result = result && checkPerft("6KQ/8/8/8/8/8/8/7k b - - 0 1", 6, 391507, perftType);
		result = result && checkPerft("K7/8/8/3Q4/4q3/8/8/7k w - - 0 1", 1, 6, perftType);
		result = result && checkPerft("K7/8/8/3Q4/4q3/8/8/7k w - - 0 1", 2, 35, perftType);
		result = result && checkPerft("K7/8/8/3Q4/4q3/8/8/7k w - - 0 1", 3, 495, perftType);
		result = result && checkPerft("K7/8/8/3Q4/4q3/8/8/7k w - - 0 1", 4, 8349, perftType);
		result = result && checkPerft("K7/8/8/3Q4/4q3/8/8/7k w - - 0 1", 5, 166741, perftType);
		result = result && checkPerft("K7/8/8/3Q4/4q3/8/8/7k w - - 0 1", 6, 3370175, perftType);
		result = result && checkPerft("6qk/8/8/8/8/8/8/7K b - - 0 1", 1, 22, perftType);
		result = result && checkPerft("6qk/8/8/8/8/8/8/7K b - - 0 1", 2, 43, perftType);
		result = result && checkPerft("6qk/8/8/8/8/8/8/7K b - - 0 1", 3, 1015, perftType);
		result = result && checkPerft("6qk/8/8/8/8/8/8/7K b - - 0 1", 4, 4167, perftType);
		result = result && checkPerft("6qk/8/8/8/8/8/8/7K b - - 0 1", 5, 105749, perftType);
		result = result && checkPerft("6qk/8/8/8/8/8/8/7K b - - 0 1", 6, 419369, perftType);
		result = result && checkPerft("6KQ/8/8/8/8/8/8/7k b - - 0 1", 1, 2, perftType);
		result = result && checkPerft("6KQ/8/8/8/8/8/8/7k b - - 0 1", 2, 36, perftType);
		result = result && checkPerft("6KQ/8/8/8/8/8/8/7k b - - 0 1", 3, 143, perftType);
		result = result && checkPerft("6KQ/8/8/8/8/8/8/7k b - - 0 1", 4, 3637, perftType);
		result = result && checkPerft("6KQ/8/8/8/8/8/8/7k b - - 0 1", 5, 14893, perftType);
		result = result && checkPerft("6KQ/8/8/8/8/8/8/7k b - - 0 1", 6, 391507, perftType);
		result = result && checkPerft("K7/8/8/3Q4/4q3/8/8/7k b - - 0 1", 1, 6, perftType);
		result = result && checkPerft("K7/8/8/3Q4/4q3/8/8/7k b - - 0 1", 2, 35, perftType);
		result = result && checkPerft("K7/8/8/3Q4/4q3/8/8/7k b - - 0 1", 3, 495, perftType);
		result = result && checkPerft("K7/8/8/3Q4/4q3/8/8/7k b - - 0 1", 4, 8349, perftType);
		result = result && checkPerft("K7/8/8/3Q4/4q3/8/8/7k b - - 0 1", 5, 166741, perftType);
		result = result && checkPerft("K7/8/8/3Q4/4q3/8/8/7k b - - 0 1", 6, 3370175, perftType);
		result = result && checkPerft("8/8/8/8/8/K7/P7/k7 w - - 0 1", 1, 3, perftType);
		result = result && checkPerft("8/8/8/8/8/K7/P7/k7 w - - 0 1", 2, 7, perftType);
		result = result && checkPerft("8/8/8/8/8/K7/P7/k7 w - - 0 1", 3, 43, perftType);
		result = result && checkPerft("8/8/8/8/8/K7/P7/k7 w - - 0 1", 4, 199, perftType);
		result = result && checkPerft("8/8/8/8/8/K7/P7/k7 w - - 0 1", 5, 1347, perftType);
		result = result && checkPerft("8/8/8/8/8/K7/P7/k7 w - - 0 1", 6, 6249, perftType);
		result = result && checkPerft("8/8/8/8/8/7K/7P/7k w - - 0 1", 1, 3, perftType);
		result = result && checkPerft("8/8/8/8/8/7K/7P/7k w - - 0 1", 2, 7, perftType);
		result = result && checkPerft("8/8/8/8/8/7K/7P/7k w - - 0 1", 3, 43, perftType);
		result = result && checkPerft("8/8/8/8/8/7K/7P/7k w - - 0 1", 4, 199, perftType);
		result = result && checkPerft("8/8/8/8/8/7K/7P/7k w - - 0 1", 5, 1347, perftType);
		result = result && checkPerft("8/8/8/8/8/7K/7P/7k w - - 0 1", 6, 6249, perftType);
		result = result && checkPerft("K7/p7/k7/8/8/8/8/8 w - - 0 1", 1, 1, perftType);
		result = result && checkPerft("K7/p7/k7/8/8/8/8/8 w - - 0 1", 2, 3, perftType);
		result = result && checkPerft("K7/p7/k7/8/8/8/8/8 w - - 0 1", 3, 12, perftType);
		result = result && checkPerft("K7/p7/k7/8/8/8/8/8 w - - 0 1", 4, 80, perftType);
		result = result && checkPerft("K7/p7/k7/8/8/8/8/8 w - - 0 1", 5, 342, perftType);
		result = result && checkPerft("K7/p7/k7/8/8/8/8/8 w - - 0 1", 6, 2343, perftType);
		result = result && checkPerft("7K/7p/7k/8/8/8/8/8 w - - 0 1", 1, 1, perftType);
		result = result && checkPerft("7K/7p/7k/8/8/8/8/8 w - - 0 1", 2, 3, perftType);
		result = result && checkPerft("7K/7p/7k/8/8/8/8/8 w - - 0 1", 3, 12, perftType);
		result = result && checkPerft("7K/7p/7k/8/8/8/8/8 w - - 0 1", 4, 80, perftType);
		result = result && checkPerft("7K/7p/7k/8/8/8/8/8 w - - 0 1", 5, 342, perftType);
		result = result && checkPerft("7K/7p/7k/8/8/8/8/8 w - - 0 1", 6, 2343, perftType);
		result = result && checkPerft("8/2k1p3/3pP3/3P2K1/8/8/8/8 w - - 0 1", 1, 7, perftType);
		result = result && checkPerft("8/2k1p3/3pP3/3P2K1/8/8/8/8 w - - 0 1", 2, 35, perftType);
		result = result && checkPerft("8/2k1p3/3pP3/3P2K1/8/8/8/8 w - - 0 1", 3, 210, perftType);
		result = result && checkPerft("8/2k1p3/3pP3/3P2K1/8/8/8/8 w - - 0 1", 4, 1091, perftType);
		result = result && checkPerft("8/2k1p3/3pP3/3P2K1/8/8/8/8 w - - 0 1", 5, 7028, perftType);
		result = result && checkPerft("8/2k1p3/3pP3/3P2K1/8/8/8/8 w - - 0 1", 6, 34834, perftType);
		result = result && checkPerft("8/8/8/8/8/K7/P7/k7 b - - 0 1", 1, 1, perftType);
		result = result && checkPerft("8/8/8/8/8/K7/P7/k7 b - - 0 1", 2, 3, perftType);
		result = result && checkPerft("8/8/8/8/8/K7/P7/k7 b - - 0 1", 3, 12, perftType);
		result = result && checkPerft("8/8/8/8/8/K7/P7/k7 b - - 0 1", 4, 80, perftType);
		result = result && checkPerft("8/8/8/8/8/K7/P7/k7 b - - 0 1", 5, 342, perftType);
		result = result && checkPerft("8/8/8/8/8/K7/P7/k7 b - - 0 1", 6, 2343, perftType);
		result = result && checkPerft("8/8/8/8/8/7K/7P/7k b - - 0 1", 1, 1, perftType);
		result = result && checkPerft("8/8/8/8/8/7K/7P/7k b - - 0 1", 2, 3, perftType);
		result = result && checkPerft("8/8/8/8/8/7K/7P/7k b - - 0 1", 3, 12, perftType);
		result = result && checkPerft("8/8/8/8/8/7K/7P/7k b - - 0 1", 4, 80, perftType);
		result = result && checkPerft("8/8/8/8/8/7K/7P/7k b - - 0 1", 5, 342, perftType);
		result = result && checkPerft("8/8/8/8/8/7K/7P/7k b - - 0 1", 6, 2343, perftType);
		result = result && checkPerft("K7/p7/k7/8/8/8/8/8 b - - 0 1", 1, 3, perftType);
		result = result && checkPerft("K7/p7/k7/8/8/8/8/8 b - - 0 1", 2, 7, perftType);
		result = result && checkPerft("K7/p7/k7/8/8/8/8/8 b - - 0 1", 3, 43, perftType);
		result = result && checkPerft("K7/p7/k7/8/8/8/8/8 b - - 0 1", 4, 199, perftType);
		result = result && checkPerft("K7/p7/k7/8/8/8/8/8 b - - 0 1", 5, 1347, perftType);
		result = result && checkPerft("K7/p7/k7/8/8/8/8/8 b - - 0 1", 6, 6249, perftType);
		result = result && checkPerft("7K/7p/7k/8/8/8/8/8 b - - 0 1", 1, 3, perftType);
		result = result && checkPerft("7K/7p/7k/8/8/8/8/8 b - - 0 1", 2, 7, perftType);
		result = result && checkPerft("7K/7p/7k/8/8/8/8/8 b - - 0 1", 3, 43, perftType);
		result = result && checkPerft("7K/7p/7k/8/8/8/8/8 b - - 0 1", 4, 199, perftType);
		result = result && checkPerft("7K/7p/7k/8/8/8/8/8 b - - 0 1", 5, 1347, perftType);
		result = result && checkPerft("7K/7p/7k/8/8/8/8/8 b - - 0 1", 6, 6249, perftType);
		result = result && checkPerft("8/2k1p3/3pP3/3P2K1/8/8/8/8 b - - 0 1", 1, 5, perftType);
		result = result && checkPerft("8/2k1p3/3pP3/3P2K1/8/8/8/8 b - - 0 1", 2, 35, perftType);
		result = result && checkPerft("8/2k1p3/3pP3/3P2K1/8/8/8/8 b - - 0 1", 3, 182, perftType);
		result = result && checkPerft("8/2k1p3/3pP3/3P2K1/8/8/8/8 b - - 0 1", 4, 1091, perftType);
		result = result && checkPerft("8/2k1p3/3pP3/3P2K1/8/8/8/8 b - - 0 1", 5, 5408, perftType);
		result = result && checkPerft("8/2k1p3/3pP3/3P2K1/8/8/8/8 b - - 0 1", 6, 34822, perftType);
		result = result && checkPerft("8/8/8/8/8/4k3/4P3/4K3 w - - 0 1", 1, 2, perftType);
		result = result && checkPerft("8/8/8/8/8/4k3/4P3/4K3 w - - 0 1", 2, 8, perftType);
		result = result && checkPerft("8/8/8/8/8/4k3/4P3/4K3 w - - 0 1", 3, 44, perftType);
		result = result && checkPerft("8/8/8/8/8/4k3/4P3/4K3 w - - 0 1", 4, 282, perftType);
		result = result && checkPerft("8/8/8/8/8/4k3/4P3/4K3 w - - 0 1", 5, 1814, perftType);
		result = result && checkPerft("8/8/8/8/8/4k3/4P3/4K3 w - - 0 1", 6, 11848, perftType);
		result = result && checkPerft("4k3/4p3/4K3/8/8/8/8/8 b - - 0 1", 1, 2, perftType);
		result = result && checkPerft("4k3/4p3/4K3/8/8/8/8/8 b - - 0 1", 2, 8, perftType);
		result = result && checkPerft("4k3/4p3/4K3/8/8/8/8/8 b - - 0 1", 3, 44, perftType);
		result = result && checkPerft("4k3/4p3/4K3/8/8/8/8/8 b - - 0 1", 4, 282, perftType);
		result = result && checkPerft("4k3/4p3/4K3/8/8/8/8/8 b - - 0 1", 5, 1814, perftType);
		result = result && checkPerft("4k3/4p3/4K3/8/8/8/8/8 b - - 0 1", 6, 11848, perftType);
		result = result && checkPerft("8/8/7k/7p/7P/7K/8/8 w - - 0 1", 1, 3, perftType);
		result = result && checkPerft("8/8/7k/7p/7P/7K/8/8 w - - 0 1", 2, 9, perftType);
		result = result && checkPerft("8/8/7k/7p/7P/7K/8/8 w - - 0 1", 3, 57, perftType);
		result = result && checkPerft("8/8/7k/7p/7P/7K/8/8 w - - 0 1", 4, 360, perftType);
		result = result && checkPerft("8/8/7k/7p/7P/7K/8/8 w - - 0 1", 5, 1969, perftType);
		result = result && checkPerft("8/8/7k/7p/7P/7K/8/8 w - - 0 1", 6, 10724, perftType);
		result = result && checkPerft("8/8/k7/p7/P7/K7/8/8 w - - 0 1", 1, 3, perftType);
		result = result && checkPerft("8/8/k7/p7/P7/K7/8/8 w - - 0 1", 2, 9, perftType);
		result = result && checkPerft("8/8/k7/p7/P7/K7/8/8 w - - 0 1", 3, 57, perftType);
		result = result && checkPerft("8/8/k7/p7/P7/K7/8/8 w - - 0 1", 4, 360, perftType);
		result = result && checkPerft("8/8/k7/p7/P7/K7/8/8 w - - 0 1", 5, 1969, perftType);
		result = result && checkPerft("8/8/k7/p7/P7/K7/8/8 w - - 0 1", 6, 10724, perftType);
		result = result && checkPerft("8/8/3k4/3p4/3P4/3K4/8/8 w - - 0 1", 1, 5, perftType);
		result = result && checkPerft("8/8/3k4/3p4/3P4/3K4/8/8 w - - 0 1", 2, 25, perftType);
		result = result && checkPerft("8/8/3k4/3p4/3P4/3K4/8/8 w - - 0 1", 3, 180, perftType);
		result = result && checkPerft("8/8/3k4/3p4/3P4/3K4/8/8 w - - 0 1", 4, 1294, perftType);
		result = result && checkPerft("8/8/3k4/3p4/3P4/3K4/8/8 w - - 0 1", 5, 8296, perftType);
		result = result && checkPerft("8/8/3k4/3p4/3P4/3K4/8/8 w - - 0 1", 6, 53138, perftType);
		result = result && checkPerft("8/3k4/3p4/8/3P4/3K4/8/8 w - - 0 1", 1, 8, perftType);
		result = result && checkPerft("8/3k4/3p4/8/3P4/3K4/8/8 w - - 0 1", 2, 61, perftType);
		result = result && checkPerft("8/3k4/3p4/8/3P4/3K4/8/8 w - - 0 1", 3, 483, perftType);
		result = result && checkPerft("8/3k4/3p4/8/3P4/3K4/8/8 w - - 0 1", 4, 3213, perftType);
		result = result && checkPerft("8/3k4/3p4/8/3P4/3K4/8/8 w - - 0 1", 5, 23599, perftType);
		result = result && checkPerft("8/3k4/3p4/8/3P4/3K4/8/8 w - - 0 1", 6, 157093, perftType);
		result = result && checkPerft("8/8/3k4/3p4/8/3P4/3K4/8 w - - 0 1", 1, 8, perftType);
		result = result && checkPerft("8/8/3k4/3p4/8/3P4/3K4/8 w - - 0 1", 2, 61, perftType);
		result = result && checkPerft("8/8/3k4/3p4/8/3P4/3K4/8 w - - 0 1", 3, 411, perftType);
		result = result && checkPerft("8/8/3k4/3p4/8/3P4/3K4/8 w - - 0 1", 4, 3213, perftType);
		result = result && checkPerft("8/8/3k4/3p4/8/3P4/3K4/8 w - - 0 1", 5, 21637, perftType);
		result = result && checkPerft("8/8/3k4/3p4/8/3P4/3K4/8 w - - 0 1", 6, 158065, perftType);
		result = result && checkPerft("k7/8/3p4/8/3P4/8/8/7K w - - 0 1", 1, 4, perftType);
		result = result && checkPerft("k7/8/3p4/8/3P4/8/8/7K w - - 0 1", 2, 15, perftType);
		result = result && checkPerft("k7/8/3p4/8/3P4/8/8/7K w - - 0 1", 3, 90, perftType);
		result = result && checkPerft("k7/8/3p4/8/3P4/8/8/7K w - - 0 1", 4, 534, perftType);
		result = result && checkPerft("k7/8/3p4/8/3P4/8/8/7K w - - 0 1", 5, 3450, perftType);
		result = result && checkPerft("k7/8/3p4/8/3P4/8/8/7K w - - 0 1", 6, 20960, perftType);
		result = result && checkPerft("8/8/7k/7p/7P/7K/8/8 b - - 0 1", 1, 3, perftType);
		result = result && checkPerft("8/8/7k/7p/7P/7K/8/8 b - - 0 1", 2, 9, perftType);
		result = result && checkPerft("8/8/7k/7p/7P/7K/8/8 b - - 0 1", 3, 57, perftType);
		result = result && checkPerft("8/8/7k/7p/7P/7K/8/8 b - - 0 1", 4, 360, perftType);
		result = result && checkPerft("8/8/7k/7p/7P/7K/8/8 b - - 0 1", 5, 1969, perftType);
		result = result && checkPerft("8/8/7k/7p/7P/7K/8/8 b - - 0 1", 6, 10724, perftType);
		result = result && checkPerft("8/8/k7/p7/P7/K7/8/8 b - - 0 1", 1, 3, perftType);
		result = result && checkPerft("8/8/k7/p7/P7/K7/8/8 b - - 0 1", 2, 9, perftType);
		result = result && checkPerft("8/8/k7/p7/P7/K7/8/8 b - - 0 1", 3, 57, perftType);
		result = result && checkPerft("8/8/k7/p7/P7/K7/8/8 b - - 0 1", 4, 360, perftType);
		result = result && checkPerft("8/8/k7/p7/P7/K7/8/8 b - - 0 1", 5, 1969, perftType);
		result = result && checkPerft("8/8/k7/p7/P7/K7/8/8 b - - 0 1", 6, 10724, perftType);
		result = result && checkPerft("8/8/3k4/3p4/3P4/3K4/8/8 b - - 0 1", 1, 5, perftType);
		result = result && checkPerft("8/8/3k4/3p4/3P4/3K4/8/8 b - - 0 1", 2, 25, perftType);
		result = result && checkPerft("8/8/3k4/3p4/3P4/3K4/8/8 b - - 0 1", 3, 180, perftType);
		result = result && checkPerft("8/8/3k4/3p4/3P4/3K4/8/8 b - - 0 1", 4, 1294, perftType);
		result = result && checkPerft("8/8/3k4/3p4/3P4/3K4/8/8 b - - 0 1", 5, 8296, perftType);
		result = result && checkPerft("8/8/3k4/3p4/3P4/3K4/8/8 b - - 0 1", 6, 53138, perftType);
		result = result && checkPerft("8/3k4/3p4/8/3P4/3K4/8/8 b - - 0 1", 1, 8, perftType);
		result = result && checkPerft("8/3k4/3p4/8/3P4/3K4/8/8 b - - 0 1", 2, 61, perftType);
		result = result && checkPerft("8/3k4/3p4/8/3P4/3K4/8/8 b - - 0 1", 3, 411, perftType);
		result = result && checkPerft("8/3k4/3p4/8/3P4/3K4/8/8 b - - 0 1", 4, 3213, perftType);
		result = result && checkPerft("8/3k4/3p4/8/3P4/3K4/8/8 b - - 0 1", 5, 21637, perftType);
		result = result && checkPerft("8/3k4/3p4/8/3P4/3K4/8/8 b - - 0 1", 6, 158065, perftType);
		result = result && checkPerft("8/8/3k4/3p4/8/3P4/3K4/8 b - - 0 1", 1, 8, perftType);
		result = result && checkPerft("8/8/3k4/3p4/8/3P4/3K4/8 b - - 0 1", 2, 61, perftType);
		result = result && checkPerft("8/8/3k4/3p4/8/3P4/3K4/8 b - - 0 1", 3, 483, perftType);
		result = result && checkPerft("8/8/3k4/3p4/8/3P4/3K4/8 b - - 0 1", 4, 3213, perftType);
		result = result && checkPerft("8/8/3k4/3p4/8/3P4/3K4/8 b - - 0 1", 5, 23599, perftType);
		result = result && checkPerft("8/8/3k4/3p4/8/3P4/3K4/8 b - - 0 1", 6, 157093, perftType);
		result = result && checkPerft("k7/8/3p4/8/3P4/8/8/7K b - - 0 1", 1, 4, perftType);
		result = result && checkPerft("k7/8/3p4/8/3P4/8/8/7K b - - 0 1", 2, 15, perftType);
		result = result && checkPerft("k7/8/3p4/8/3P4/8/8/7K b - - 0 1", 3, 89, perftType);
		result = result && checkPerft("k7/8/3p4/8/3P4/8/8/7K b - - 0 1", 4, 537, perftType);
		result = result && checkPerft("k7/8/3p4/8/3P4/8/8/7K b - - 0 1", 5, 3309, perftType);
		result = result && checkPerft("k7/8/3p4/8/3P4/8/8/7K b - - 0 1", 6, 21104, perftType);
		result = result && checkPerft("7k/3p4/8/8/3P4/8/8/K7 w - - 0 1", 1, 4, perftType);
		result = result && checkPerft("7k/3p4/8/8/3P4/8/8/K7 w - - 0 1", 2, 19, perftType);
		result = result && checkPerft("7k/3p4/8/8/3P4/8/8/K7 w - - 0 1", 3, 117, perftType);
		result = result && checkPerft("7k/3p4/8/8/3P4/8/8/K7 w - - 0 1", 4, 720, perftType);
		result = result && checkPerft("7k/3p4/8/8/3P4/8/8/K7 w - - 0 1", 5, 4661, perftType);
		result = result && checkPerft("7k/3p4/8/8/3P4/8/8/K7 w - - 0 1", 6, 32191, perftType);
		result = result && checkPerft("7k/8/8/3p4/8/8/3P4/K7 w - - 0 1", 1, 5, perftType);
		result = result && checkPerft("7k/8/8/3p4/8/8/3P4/K7 w - - 0 1", 2, 19, perftType);
		result = result && checkPerft("7k/8/8/3p4/8/8/3P4/K7 w - - 0 1", 3, 116, perftType);
		result = result && checkPerft("7k/8/8/3p4/8/8/3P4/K7 w - - 0 1", 4, 716, perftType);
		result = result && checkPerft("7k/8/8/3p4/8/8/3P4/K7 w - - 0 1", 5, 4786, perftType);
		result = result && checkPerft("7k/8/8/3p4/8/8/3P4/K7 w - - 0 1", 6, 30980, perftType);
		result = result && checkPerft("k7/8/8/7p/6P1/8/8/K7 w - - 0 1", 1, 5, perftType);
		result = result && checkPerft("k7/8/8/7p/6P1/8/8/K7 w - - 0 1", 2, 22, perftType);
		result = result && checkPerft("k7/8/8/7p/6P1/8/8/K7 w - - 0 1", 3, 139, perftType);
		result = result && checkPerft("k7/8/8/7p/6P1/8/8/K7 w - - 0 1", 4, 877, perftType);
		result = result && checkPerft("k7/8/8/7p/6P1/8/8/K7 w - - 0 1", 5, 6112, perftType);
		result = result && checkPerft("k7/8/8/7p/6P1/8/8/K7 w - - 0 1", 6, 41874, perftType);
		result = result && checkPerft("k7/8/7p/8/8/6P1/8/K7 w - - 0 1", 1, 4, perftType);
		result = result && checkPerft("k7/8/7p/8/8/6P1/8/K7 w - - 0 1", 2, 16, perftType);
		result = result && checkPerft("k7/8/7p/8/8/6P1/8/K7 w - - 0 1", 3, 101, perftType);
		result = result && checkPerft("k7/8/7p/8/8/6P1/8/K7 w - - 0 1", 4, 637, perftType);
		result = result && checkPerft("k7/8/7p/8/8/6P1/8/K7 w - - 0 1", 5, 4354, perftType);
		result = result && checkPerft("k7/8/7p/8/8/6P1/8/K7 w - - 0 1", 6, 29679, perftType);
		result = result && checkPerft("k7/8/8/6p1/7P/8/8/K7 w - - 0 1", 1, 5, perftType);
		result = result && checkPerft("k7/8/8/6p1/7P/8/8/K7 w - - 0 1", 2, 22, perftType);
		result = result && checkPerft("k7/8/8/6p1/7P/8/8/K7 w - - 0 1", 3, 139, perftType);
		result = result && checkPerft("k7/8/8/6p1/7P/8/8/K7 w - - 0 1", 4, 877, perftType);
		result = result && checkPerft("k7/8/8/6p1/7P/8/8/K7 w - - 0 1", 5, 6112, perftType);
		result = result && checkPerft("k7/8/8/6p1/7P/8/8/K7 w - - 0 1", 6, 41874, perftType);
		result = result && checkPerft("k7/8/6p1/8/8/7P/8/K7 w - - 0 1", 1, 4, perftType);
		result = result && checkPerft("k7/8/6p1/8/8/7P/8/K7 w - - 0 1", 2, 16, perftType);
		result = result && checkPerft("k7/8/6p1/8/8/7P/8/K7 w - - 0 1", 3, 101, perftType);
		result = result && checkPerft("k7/8/6p1/8/8/7P/8/K7 w - - 0 1", 4, 637, perftType);
		result = result && checkPerft("k7/8/6p1/8/8/7P/8/K7 w - - 0 1", 5, 4354, perftType);
		result = result && checkPerft("k7/8/6p1/8/8/7P/8/K7 w - - 0 1", 6, 29679, perftType);
		result = result && checkPerft("k7/8/8/3p4/4p3/8/8/7K w - - 0 1", 1, 3, perftType);
		result = result && checkPerft("k7/8/8/3p4/4p3/8/8/7K w - - 0 1", 2, 15, perftType);
		result = result && checkPerft("k7/8/8/3p4/4p3/8/8/7K w - - 0 1", 3, 84, perftType);
		result = result && checkPerft("k7/8/8/3p4/4p3/8/8/7K w - - 0 1", 4, 573, perftType);
		result = result && checkPerft("k7/8/8/3p4/4p3/8/8/7K w - - 0 1", 5, 3013, perftType);
		result = result && checkPerft("k7/8/8/3p4/4p3/8/8/7K w - - 0 1", 6, 22886, perftType);
		result = result && checkPerft("k7/8/3p4/8/8/4P3/8/7K w - - 0 1", 1, 4, perftType);
		result = result && checkPerft("k7/8/3p4/8/8/4P3/8/7K w - - 0 1", 2, 16, perftType);
		result = result && checkPerft("k7/8/3p4/8/8/4P3/8/7K w - - 0 1", 3, 101, perftType);
		result = result && checkPerft("k7/8/3p4/8/8/4P3/8/7K w - - 0 1", 4, 637, perftType);
		result = result && checkPerft("k7/8/3p4/8/8/4P3/8/7K w - - 0 1", 5, 4271, perftType);
		result = result && checkPerft("k7/8/3p4/8/8/4P3/8/7K w - - 0 1", 6, 28662, perftType);
		result = result && checkPerft("7k/3p4/8/8/3P4/8/8/K7 b - - 0 1", 1, 5, perftType);
		result = result && checkPerft("7k/3p4/8/8/3P4/8/8/K7 b - - 0 1", 2, 19, perftType);
		result = result && checkPerft("7k/3p4/8/8/3P4/8/8/K7 b - - 0 1", 3, 117, perftType);
		result = result && checkPerft("7k/3p4/8/8/3P4/8/8/K7 b - - 0 1", 4, 720, perftType);
		result = result && checkPerft("7k/3p4/8/8/3P4/8/8/K7 b - - 0 1", 5, 5014, perftType);
		result = result && checkPerft("7k/3p4/8/8/3P4/8/8/K7 b - - 0 1", 6, 32167, perftType);
		result = result && checkPerft("7k/8/8/3p4/8/8/3P4/K7 b - - 0 1", 1, 4, perftType);
		result = result && checkPerft("7k/8/8/3p4/8/8/3P4/K7 b - - 0 1", 2, 19, perftType);
		result = result && checkPerft("7k/8/8/3p4/8/8/3P4/K7 b - - 0 1", 3, 117, perftType);
		result = result && checkPerft("7k/8/8/3p4/8/8/3P4/K7 b - - 0 1", 4, 712, perftType);
		result = result && checkPerft("7k/8/8/3p4/8/8/3P4/K7 b - - 0 1", 5, 4658, perftType);
		result = result && checkPerft("7k/8/8/3p4/8/8/3P4/K7 b - - 0 1", 6, 30749, perftType);
		result = result && checkPerft("k7/8/8/7p/6P1/8/8/K7 b - - 0 1", 1, 5, perftType);
		result = result && checkPerft("k7/8/8/7p/6P1/8/8/K7 b - - 0 1", 2, 22, perftType);
		result = result && checkPerft("k7/8/8/7p/6P1/8/8/K7 b - - 0 1", 3, 139, perftType);
		result = result && checkPerft("k7/8/8/7p/6P1/8/8/K7 b - - 0 1", 4, 877, perftType);
		result = result && checkPerft("k7/8/8/7p/6P1/8/8/K7 b - - 0 1", 5, 6112, perftType);
		result = result && checkPerft("k7/8/8/7p/6P1/8/8/K7 b - - 0 1", 6, 41874, perftType);
		result = result && checkPerft("k7/8/7p/8/8/6P1/8/K7 b - - 0 1", 1, 4, perftType);
		result = result && checkPerft("k7/8/7p/8/8/6P1/8/K7 b - - 0 1", 2, 16, perftType);
		result = result && checkPerft("k7/8/7p/8/8/6P1/8/K7 b - - 0 1", 3, 101, perftType);
		result = result && checkPerft("k7/8/7p/8/8/6P1/8/K7 b - - 0 1", 4, 637, perftType);
		result = result && checkPerft("k7/8/7p/8/8/6P1/8/K7 b - - 0 1", 5, 4354, perftType);
		result = result && checkPerft("k7/8/7p/8/8/6P1/8/K7 b - - 0 1", 6, 29679, perftType);
		result = result && checkPerft("k7/8/8/6p1/7P/8/8/K7 b - - 0 1", 1, 5, perftType);
		result = result && checkPerft("k7/8/8/6p1/7P/8/8/K7 b - - 0 1", 2, 22, perftType);
		result = result && checkPerft("k7/8/8/6p1/7P/8/8/K7 b - - 0 1", 3, 139, perftType);
		result = result && checkPerft("k7/8/8/6p1/7P/8/8/K7 b - - 0 1", 4, 877, perftType);
		result = result && checkPerft("k7/8/8/6p1/7P/8/8/K7 b - - 0 1", 5, 6112, perftType);
		result = result && checkPerft("k7/8/8/6p1/7P/8/8/K7 b - - 0 1", 6, 41874, perftType);
		result = result && checkPerft("k7/8/6p1/8/8/7P/8/K7 b - - 0 1", 1, 4, perftType);
		result = result && checkPerft("k7/8/6p1/8/8/7P/8/K7 b - - 0 1", 2, 16, perftType);
		result = result && checkPerft("k7/8/6p1/8/8/7P/8/K7 b - - 0 1", 3, 101, perftType);
		result = result && checkPerft("k7/8/6p1/8/8/7P/8/K7 b - - 0 1", 4, 637, perftType);
		result = result && checkPerft("k7/8/6p1/8/8/7P/8/K7 b - - 0 1", 5, 4354, perftType);
		result = result && checkPerft("k7/8/6p1/8/8/7P/8/K7 b - - 0 1", 6, 29679, perftType);
		result = result && checkPerft("k7/8/8/3p4/4p3/8/8/7K b - - 0 1", 1, 5, perftType);
		result = result && checkPerft("k7/8/8/3p4/4p3/8/8/7K b - - 0 1", 2, 15, perftType);
		result = result && checkPerft("k7/8/8/3p4/4p3/8/8/7K b - - 0 1", 3, 102, perftType);
		result = result && checkPerft("k7/8/8/3p4/4p3/8/8/7K b - - 0 1", 4, 569, perftType);
		result = result && checkPerft("k7/8/8/3p4/4p3/8/8/7K b - - 0 1", 5, 4337, perftType);
		result = result && checkPerft("k7/8/8/3p4/4p3/8/8/7K b - - 0 1", 6, 22579, perftType);
		result = result && checkPerft("k7/8/3p4/8/8/4P3/8/7K b - - 0 1", 1, 4, perftType);
		result = result && checkPerft("k7/8/3p4/8/8/4P3/8/7K b - - 0 1", 2, 16, perftType);
		result = result && checkPerft("k7/8/3p4/8/8/4P3/8/7K b - - 0 1", 3, 101, perftType);
		result = result && checkPerft("k7/8/3p4/8/8/4P3/8/7K b - - 0 1", 4, 637, perftType);
		result = result && checkPerft("k7/8/3p4/8/8/4P3/8/7K b - - 0 1", 5, 4271, perftType);
		result = result && checkPerft("k7/8/3p4/8/8/4P3/8/7K b - - 0 1", 6, 28662, perftType);
		result = result && checkPerft("7k/8/8/p7/1P6/8/8/7K w - - 0 1", 1, 5, perftType);
		result = result && checkPerft("7k/8/8/p7/1P6/8/8/7K w - - 0 1", 2, 22, perftType);
		result = result && checkPerft("7k/8/8/p7/1P6/8/8/7K w - - 0 1", 3, 139, perftType);
		result = result && checkPerft("7k/8/8/p7/1P6/8/8/7K w - - 0 1", 4, 877, perftType);
		result = result && checkPerft("7k/8/8/p7/1P6/8/8/7K w - - 0 1", 5, 6112, perftType);
		result = result && checkPerft("7k/8/8/p7/1P6/8/8/7K w - - 0 1", 6, 41874, perftType);
		result = result && checkPerft("7k/8/p7/8/8/1P6/8/7K w - - 0 1", 1, 4, perftType);
		result = result && checkPerft("7k/8/p7/8/8/1P6/8/7K w - - 0 1", 2, 16, perftType);
		result = result && checkPerft("7k/8/p7/8/8/1P6/8/7K w - - 0 1", 3, 101, perftType);
		result = result && checkPerft("7k/8/p7/8/8/1P6/8/7K w - - 0 1", 4, 637, perftType);
		result = result && checkPerft("7k/8/p7/8/8/1P6/8/7K w - - 0 1", 5, 4354, perftType);
		result = result && checkPerft("7k/8/p7/8/8/1P6/8/7K w - - 0 1", 6, 29679, perftType);
		result = result && checkPerft("7k/8/8/1p6/P7/8/8/7K w - - 0 1", 1, 5, perftType);
		result = result && checkPerft("7k/8/8/1p6/P7/8/8/7K w - - 0 1", 2, 22, perftType);
		result = result && checkPerft("7k/8/8/1p6/P7/8/8/7K w - - 0 1", 3, 139, perftType);
		result = result && checkPerft("7k/8/8/1p6/P7/8/8/7K w - - 0 1", 4, 877, perftType);
		result = result && checkPerft("7k/8/8/1p6/P7/8/8/7K w - - 0 1", 5, 6112, perftType);
		result = result && checkPerft("7k/8/8/1p6/P7/8/8/7K w - - 0 1", 6, 41874, perftType);
		result = result && checkPerft("7k/8/1p6/8/8/P7/8/7K w - - 0 1", 1, 4, perftType);
		result = result && checkPerft("7k/8/1p6/8/8/P7/8/7K w - - 0 1", 2, 16, perftType);
		result = result && checkPerft("7k/8/1p6/8/8/P7/8/7K w - - 0 1", 3, 101, perftType);
		result = result && checkPerft("7k/8/1p6/8/8/P7/8/7K w - - 0 1", 4, 637, perftType);
		result = result && checkPerft("7k/8/1p6/8/8/P7/8/7K w - - 0 1", 5, 4354, perftType);
		result = result && checkPerft("7k/8/1p6/8/8/P7/8/7K w - - 0 1", 6, 29679, perftType);
		result = result && checkPerft("k7/7p/8/8/8/8/6P1/K7 w - - 0 1", 1, 5, perftType);
		result = result && checkPerft("k7/7p/8/8/8/8/6P1/K7 w - - 0 1", 2, 25, perftType);
		result = result && checkPerft("k7/7p/8/8/8/8/6P1/K7 w - - 0 1", 3, 161, perftType);
		result = result && checkPerft("k7/7p/8/8/8/8/6P1/K7 w - - 0 1", 4, 1035, perftType);
		result = result && checkPerft("k7/7p/8/8/8/8/6P1/K7 w - - 0 1", 5, 7574, perftType);
		result = result && checkPerft("k7/7p/8/8/8/8/6P1/K7 w - - 0 1", 6, 55338, perftType);
		result = result && checkPerft("k7/6p1/8/8/8/8/7P/K7 w - - 0 1", 1, 5, perftType);
		result = result && checkPerft("k7/6p1/8/8/8/8/7P/K7 w - - 0 1", 2, 25, perftType);
		result = result && checkPerft("k7/6p1/8/8/8/8/7P/K7 w - - 0 1", 3, 161, perftType);
		result = result && checkPerft("k7/6p1/8/8/8/8/7P/K7 w - - 0 1", 4, 1035, perftType);
		result = result && checkPerft("k7/6p1/8/8/8/8/7P/K7 w - - 0 1", 5, 7574, perftType);
		result = result && checkPerft("k7/6p1/8/8/8/8/7P/K7 w - - 0 1", 6, 55338, perftType);
		result = result && checkPerft("3k4/3pp3/8/8/8/8/3PP3/3K4 w - - 0 1", 1, 7, perftType);
		result = result && checkPerft("3k4/3pp3/8/8/8/8/3PP3/3K4 w - - 0 1", 2, 49, perftType);
		result = result && checkPerft("3k4/3pp3/8/8/8/8/3PP3/3K4 w - - 0 1", 3, 378, perftType);
		result = result && checkPerft("3k4/3pp3/8/8/8/8/3PP3/3K4 w - - 0 1", 4, 2902, perftType);
		result = result && checkPerft("3k4/3pp3/8/8/8/8/3PP3/3K4 w - - 0 1", 5, 24122, perftType);
		result = result && checkPerft("3k4/3pp3/8/8/8/8/3PP3/3K4 w - - 0 1", 6, 199002, perftType);
		result = result && checkPerft("7k/8/8/p7/1P6/8/8/7K b - - 0 1", 1, 5, perftType);
		result = result && checkPerft("7k/8/8/p7/1P6/8/8/7K b - - 0 1", 2, 22, perftType);
		result = result && checkPerft("7k/8/8/p7/1P6/8/8/7K b - - 0 1", 3, 139, perftType);
		result = result && checkPerft("7k/8/8/p7/1P6/8/8/7K b - - 0 1", 4, 877, perftType);
		result = result && checkPerft("7k/8/8/p7/1P6/8/8/7K b - - 0 1", 5, 6112, perftType);
		result = result && checkPerft("7k/8/8/p7/1P6/8/8/7K b - - 0 1", 6, 41874, perftType);
		result = result && checkPerft("7k/8/p7/8/8/1P6/8/7K b - - 0 1", 1, 4, perftType);
		result = result && checkPerft("7k/8/p7/8/8/1P6/8/7K b - - 0 1", 2, 16, perftType);
		result = result && checkPerft("7k/8/p7/8/8/1P6/8/7K b - - 0 1", 3, 101, perftType);
		result = result && checkPerft("7k/8/p7/8/8/1P6/8/7K b - - 0 1", 4, 637, perftType);
		result = result && checkPerft("7k/8/p7/8/8/1P6/8/7K b - - 0 1", 5, 4354, perftType);
		result = result && checkPerft("7k/8/p7/8/8/1P6/8/7K b - - 0 1", 6, 29679, perftType);
		result = result && checkPerft("7k/8/8/1p6/P7/8/8/7K b - - 0 1", 1, 5, perftType);
		result = result && checkPerft("7k/8/8/1p6/P7/8/8/7K b - - 0 1", 2, 22, perftType);
		result = result && checkPerft("7k/8/8/1p6/P7/8/8/7K b - - 0 1", 3, 139, perftType);
		result = result && checkPerft("7k/8/8/1p6/P7/8/8/7K b - - 0 1", 4, 877, perftType);
		result = result && checkPerft("7k/8/8/1p6/P7/8/8/7K b - - 0 1", 5, 6112, perftType);
		result = result && checkPerft("7k/8/8/1p6/P7/8/8/7K b - - 0 1", 6, 41874, perftType);
		result = result && checkPerft("7k/8/1p6/8/8/P7/8/7K b - - 0 1", 1, 4, perftType);
		result = result && checkPerft("7k/8/1p6/8/8/P7/8/7K b - - 0 1", 2, 16, perftType);
		result = result && checkPerft("7k/8/1p6/8/8/P7/8/7K b - - 0 1", 3, 101, perftType);
		result = result && checkPerft("7k/8/1p6/8/8/P7/8/7K b - - 0 1", 4, 637, perftType);
		result = result && checkPerft("7k/8/1p6/8/8/P7/8/7K b - - 0 1", 5, 4354, perftType);
		result = result && checkPerft("7k/8/1p6/8/8/P7/8/7K b - - 0 1", 6, 29679, perftType);
		result = result && checkPerft("k7/7p/8/8/8/8/6P1/K7 b - - 0 1", 1, 5, perftType);
		result = result && checkPerft("k7/7p/8/8/8/8/6P1/K7 b - - 0 1", 2, 25, perftType);
		result = result && checkPerft("k7/7p/8/8/8/8/6P1/K7 b - - 0 1", 3, 161, perftType);
		result = result && checkPerft("k7/7p/8/8/8/8/6P1/K7 b - - 0 1", 4, 1035, perftType);
		result = result && checkPerft("k7/7p/8/8/8/8/6P1/K7 b - - 0 1", 5, 7574, perftType);
		result = result && checkPerft("k7/7p/8/8/8/8/6P1/K7 b - - 0 1", 6, 55338, perftType);
		result = result && checkPerft("k7/6p1/8/8/8/8/7P/K7 b - - 0 1", 1, 5, perftType);
		result = result && checkPerft("k7/6p1/8/8/8/8/7P/K7 b - - 0 1", 2, 25, perftType);
		result = result && checkPerft("k7/6p1/8/8/8/8/7P/K7 b - - 0 1", 3, 161, perftType);
		result = result && checkPerft("k7/6p1/8/8/8/8/7P/K7 b - - 0 1", 4, 1035, perftType);
		result = result && checkPerft("k7/6p1/8/8/8/8/7P/K7 b - - 0 1", 5, 7574, perftType);
		result = result && checkPerft("k7/6p1/8/8/8/8/7P/K7 b - - 0 1", 6, 55338, perftType);
		result = result && checkPerft("3k4/3pp3/8/8/8/8/3PP3/3K4 b - - 0 1", 1, 7, perftType);
		result = result && checkPerft("3k4/3pp3/8/8/8/8/3PP3/3K4 b - - 0 1", 2, 49, perftType);
		result = result && checkPerft("3k4/3pp3/8/8/8/8/3PP3/3K4 b - - 0 1", 3, 378, perftType);
		result = result && checkPerft("3k4/3pp3/8/8/8/8/3PP3/3K4 b - - 0 1", 4, 2902, perftType);
		result = result && checkPerft("3k4/3pp3/8/8/8/8/3PP3/3K4 b - - 0 1", 5, 24122, perftType);
		result = result && checkPerft("3k4/3pp3/8/8/8/8/3PP3/3K4 b - - 0 1", 6, 199002, perftType);
		result = result && checkPerft("8/Pk6/8/8/8/8/6Kp/8 w - - 0 1", 1, 11, perftType);
		result = result && checkPerft("8/Pk6/8/8/8/8/6Kp/8 w - - 0 1", 2, 97, perftType);
		result = result && checkPerft("8/Pk6/8/8/8/8/6Kp/8 w - - 0 1", 3, 887, perftType);
		result = result && checkPerft("8/Pk6/8/8/8/8/6Kp/8 w - - 0 1", 4, 8048, perftType);
		result = result && checkPerft("8/Pk6/8/8/8/8/6Kp/8 w - - 0 1", 5, 90606, perftType);
		result = result && checkPerft("8/Pk6/8/8/8/8/6Kp/8 w - - 0 1", 6, 1030499, perftType);
		result = result && checkPerft("n1n5/1Pk5/8/8/8/8/5Kp1/5N1N w - - 0 1", 1, 24, perftType);
		result = result && checkPerft("n1n5/1Pk5/8/8/8/8/5Kp1/5N1N w - - 0 1", 2, 421, perftType);
		result = result && checkPerft("n1n5/1Pk5/8/8/8/8/5Kp1/5N1N w - - 0 1", 3, 7421, perftType);
		result = result && checkPerft("n1n5/1Pk5/8/8/8/8/5Kp1/5N1N w - - 0 1", 4, 124608, perftType);
		result = result && checkPerft("n1n5/1Pk5/8/8/8/8/5Kp1/5N1N w - - 0 1", 5, 2193768, perftType);
		result = result && checkPerft("n1n5/1Pk5/8/8/8/8/5Kp1/5N1N w - - 0 1", 6, 37665329, perftType);
		result = result && checkPerft("8/PPPk4/8/8/8/8/4Kppp/8 w - - 0 1", 1, 18, perftType);
		result = result && checkPerft("8/PPPk4/8/8/8/8/4Kppp/8 w - - 0 1", 2, 270, perftType);
		result = result && checkPerft("8/PPPk4/8/8/8/8/4Kppp/8 w - - 0 1", 3, 4699, perftType);
		result = result && checkPerft("8/PPPk4/8/8/8/8/4Kppp/8 w - - 0 1", 4, 79355, perftType);
		result = result && checkPerft("8/PPPk4/8/8/8/8/4Kppp/8 w - - 0 1", 5, 1533145, perftType);
		result = result && checkPerft("8/PPPk4/8/8/8/8/4Kppp/8 w - - 0 1", 6, 28859283, perftType);
		result = result && checkPerft("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1", 1, 24, perftType);
		result = result && checkPerft("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1", 2, 496, perftType);
		result = result && checkPerft("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1", 3, 9483, perftType);
		result = result && checkPerft("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1", 4, 182838, perftType);
		result = result && checkPerft("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1", 5, 3605103, perftType);
		result = result && checkPerft("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1", 6, 71179139, perftType);
		result = result && checkPerft("8/Pk6/8/8/8/8/6Kp/8 b - - 0 1", 1, 11, perftType);
		result = result && checkPerft("8/Pk6/8/8/8/8/6Kp/8 b - - 0 1", 2, 97, perftType);
		result = result && checkPerft("8/Pk6/8/8/8/8/6Kp/8 b - - 0 1", 3, 887, perftType);
		result = result && checkPerft("8/Pk6/8/8/8/8/6Kp/8 b - - 0 1", 4, 8048, perftType);
		result = result && checkPerft("8/Pk6/8/8/8/8/6Kp/8 b - - 0 1", 5, 90606, perftType);
		result = result && checkPerft("8/Pk6/8/8/8/8/6Kp/8 b - - 0 1", 6, 1030499, perftType);
		result = result && checkPerft("n1n5/1Pk5/8/8/8/8/5Kp1/5N1N b - - 0 1", 1, 24, perftType);
		result = result && checkPerft("n1n5/1Pk5/8/8/8/8/5Kp1/5N1N b - - 0 1", 2, 421, perftType);
		result = result && checkPerft("n1n5/1Pk5/8/8/8/8/5Kp1/5N1N b - - 0 1", 3, 7421, perftType);
		result = result && checkPerft("n1n5/1Pk5/8/8/8/8/5Kp1/5N1N b - - 0 1", 4, 124608, perftType);
		result = result && checkPerft("n1n5/1Pk5/8/8/8/8/5Kp1/5N1N b - - 0 1", 5, 2193768, perftType);
		result = result && checkPerft("n1n5/1Pk5/8/8/8/8/5Kp1/5N1N b - - 0 1", 6, 37665329, perftType);
		result = result && checkPerft("8/PPPk4/8/8/8/8/4Kppp/8 b - - 0 1", 1, 18, perftType);
		result = result && checkPerft("8/PPPk4/8/8/8/8/4Kppp/8 b - - 0 1", 2, 270, perftType);
		result = result && checkPerft("8/PPPk4/8/8/8/8/4Kppp/8 b - - 0 1", 3, 4699, perftType);
		result = result && checkPerft("8/PPPk4/8/8/8/8/4Kppp/8 b - - 0 1", 4, 79355, perftType);
		result = result && checkPerft("8/PPPk4/8/8/8/8/4Kppp/8 b - - 0 1", 5, 1533145, perftType);
		result = result && checkPerft("8/PPPk4/8/8/8/8/4Kppp/8 b - - 0 1", 6, 28859283, perftType);
		result = result && checkPerft("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1", 1, 24, perftType);
		result = result && checkPerft("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1", 2, 496, perftType);
		result = result && checkPerft("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1", 3, 9483, perftType);
		result = result && checkPerft("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1", 4, 182838, perftType);
		result = result && checkPerft("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1", 5, 3605103, perftType);
		result = result && checkPerft("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1", 6, 71179139, perftType);
		if (result)std::cout << "Done OK" << std::endl; else std::cout << "Error!" << std::endl;
		std::cout << "Runtime: " << std::setprecision(3) << perftRuntime / 1000.0 << " s" << std::endl;
		std::cout << "Count:   " << std::setprecision(3) << perftNodes / 1000000.0 << " MNodes" << std::endl;
		std::cout << "Leafs: " << std::setprecision(3) << (1.0 * perftNodes) / perftRuntime / 1000 << " MNodes/s" << std::endl;
		std::cout << "Nodes: " << std::setprecision(3) << (1.0 * nodeCount) / perftRuntime / 1000 << " MNodes/s" << std::endl;
		return result;
	}

	bool testSEE() {
		std::vector<std::string> tests = {
			"r6r/p1p1qpb1/Q4np1/8/4k3/2B4p/PPP1KPPP/R6R w - - 0 1;a6d3;0",
			"8/8/2Kp4/8/3k4/8/8/8 w - - 0 1;c6d6;80",
			"8/8/2Kp4/8/3k4/6b1/8/8 w - - 0 1;c6d6;-9920",
			"3k4/4q3/4r3/4r3/4Q3/4R3/4R3/3K4 w - - 0 1;e4e5;30",
			"3k4/4q3/4r3/4r3/4Q3/4R3/4R3/3K4 w - - 0 1;e5e4;950",
			"3k4/4q3/4R3/4r3/4Q3/4r3/4R3/3K4 w - - 0 1;e6e7;470"
		};
		bool result = true;
		for (int i = 0; i < gsl::narrow_cast<int>(tests.size()); i++) {
			auto tokens = utils::split(tests.at(i), ';');
			assert(tokens.size() == 3);
			Position pos(tokens.at(0));
			const Move move = parseMoveInUCINotation(tokens.at(1), pos);
			std::cout << tokens.at(0) << std::endl;
			const Value vsee = pos.SEE(move);
			const bool lresult = vsee == std::stoi(tokens.at(2));
			result = result && lresult;
			if (!lresult) std::cout << "ERROR ";
			std::cout << tokens.at(1) << "\t" << static_cast<int>(vsee) << "\t" << tokens.at(2) << std::endl;
		}
		return result;
	}

	std::vector<std::string> readTextFile(std::string file) {
		std::vector<std::string> lines;
		std::ifstream s;
		s.open(file);
		std::string line;
		if (s.is_open())
		{
			long lineCount = 0;
			while (s)
			{
				std::getline(s, line);
				lines.push_back(line);
				lineCount++;
				if ((lineCount & 8191) == 0)std::cout << ".";
			}
			s.close();
			std::cout << std::endl;
			std::cout << lineCount << " lines read!" << std::endl;
		}
		else
		{
			std::cout << "Unable to open file" << std::endl;
		}
		return lines;
	}

}