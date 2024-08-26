#include <string>
#include <string_view>
#include <vector>
#include <iostream>
#include "../engine.h"
#include "minunit.h"


// these fens come from gigantua's move generator test
// https://github.com/Gigantua/Gigantua/tree/main/Gigantua
// they seem to be carefully chosen to deal with tons of edge cases
static constexpr std::string_view GIGANTUA_FENS[128]
{
	"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
	"4k3/8/8/8/8/8/8/4K2R w K - 0 1", "4k3/8/8/8/8/8/8/R3K3 w Q - 0 1", "4k2r/8/8/8/8/8/8/4K3 w k - 0 1", "r3k3/8/8/8/8/8/8/4K3 w q - 0 1",
	"4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1", "r3k2r/8/8/8/8/8/8/4K3 w kq - 0 1", "8/8/8/8/8/8/6k1/4K2R w K - 0 1", "8/8/8/8/8/8/1k6/R3K3 w Q - 0 1",
	"4k2r/6K1/8/8/8/8/8/8 w k - 0 1", "r3k3/1K6/8/8/8/8/8/8 w q - 0 1", "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", "r3k2r/8/8/8/8/8/8/1R2K2R w Kkq - 0 1",
	"r3k2r/8/8/8/8/8/8/2R1K2R w Kkq - 0 1", "r3k2r/8/8/8/8/8/8/R3K1R1 w Qkq - 0 1", "1r2k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1", "2r1k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1",
	"r3k1r1/8/8/8/8/8/8/R3K2R w KQq - 0 1", "4k3/8/8/8/8/8/8/4K2R b K - 0 1", "4k3/8/8/8/8/8/8/R3K3 b Q - 0 1", "4k2r/8/8/8/8/8/8/4K3 b k - 0 1",
	"r3k3/8/8/8/8/8/8/4K3 b q - 0 1", "4k3/8/8/8/8/8/8/R3K2R b KQ - 0 1", "r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1", "8/8/8/8/8/8/6k1/4K2R b K - 0 1",
	"8/8/8/8/8/8/1k6/R3K3 b Q - 0 1", "4k2r/6K1/8/8/8/8/8/8 b k - 0 1", "r3k3/1K6/8/8/8/8/8/8 b q - 0 1", "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1",
	"r3k2r/8/8/8/8/8/8/1R2K2R b Kkq - 0 1", "r3k2r/8/8/8/8/8/8/2R1K2R b Kkq - 0 1", "r3k2r/8/8/8/8/8/8/R3K1R1 b Qkq - 0 1", "1r2k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1",
	"2r1k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1", "r3k1r1/8/8/8/8/8/8/R3K2R b KQq - 0 1", "8/1n4N1/2k5/8/8/5K2/1N4n1/8 w - - 0 1", "8/1k6/8/5N2/8/4n3/8/2K5 w - - 0 1",
	"8/8/4k3/3Nn3/3nN3/4K3/8/8 w - - 0 1", "K7/8/2n5/1n6/8/8/8/k6N w - - 0 1", "k7/8/2N5/1N6/8/8/8/K6n w - - 0 1", "8/1n4N1/2k5/8/8/5K2/1N4n1/8 b - - 0 1",
	"8/1k6/8/5N2/8/4n3/8/2K5 b - - 0 1", "8/8/3K4/3Nn3/3nN3/4k3/8/8 b - - 0 1", "K7/8/2n5/1n6/8/8/8/k6N b - - 0 1", "k7/8/2N5/1N6/8/8/8/K6n b - - 0 1",
	"B6b/8/8/8/2K5/4k3/8/b6B w - - 0 1", "8/8/1B6/7b/7k/8/2B1b3/7K w - - 0 1", "k7/B7/1B6/1B6/8/8/8/K6b w - - 0 1", "K7/b7/1b6/1b6/8/8/8/k6B w - - 0 1",
	"B6b/8/8/8/2K5/5k2/8/b6B b - - 0 1", "8/8/1B6/7b/7k/8/2B1b3/7K b - - 0 1", "k7/B7/1B6/1B6/8/8/8/K6b b - - 0 1", "K7/b7/1b6/1b6/8/8/8/k6B b - - 0 1",
	"7k/RR6/8/8/8/8/rr6/7K w - - 0 1", "R6r/8/8/2K5/5k2/8/8/r6R w - - 0 1", "7k/RR6/8/8/8/8/rr6/7K b - - 0 1", "R6r/8/8/2K5/5k2/8/8/r6R b - - 0 1",
	"6kq/8/8/8/8/8/8/7K w - - 0 1", "6KQ/8/8/8/8/8/8/7k b - - 0 1", "K7/8/8/3Q4/4q3/8/8/7k w - - 0 1", "6qk/8/8/8/8/8/8/7K b - - 0 1",
	"6KQ/8/8/8/8/8/8/7k b - - 0 1", "K7/8/8/3Q4/4q3/8/8/7k b - - 0 1", "8/8/8/8/8/K7/P7/k7 w - - 0 1", "8/8/8/8/8/7K/7P/7k w - - 0 1",
	"K7/p7/k7/8/8/8/8/8 w - - 0 1", "7K/7p/7k/8/8/8/8/8 w - - 0 1", "8/2k1p3/3pP3/3P2K1/8/8/8/8 w - - 0 1", "8/8/8/8/8/K7/P7/k7 b - - 0 1",
	"8/8/8/8/8/7K/7P/7k b - - 0 1", "K7/p7/k7/8/8/8/8/8 b - - 0 1", "7K/7p/7k/8/8/8/8/8 b - - 0 1", "8/2k1p3/3pP3/3P2K1/8/8/8/8 b - - 0 1",
	"8/8/8/8/8/4k3/4P3/4K3 w - - 0 1", "4k3/4p3/4K3/8/8/8/8/8 b - - 0 1", "8/8/7k/7p/7P/7K/8/8 w - - 0 1", "8/8/k7/p7/P7/K7/8/8 w - - 0 1",
	"8/8/3k4/3p4/3P4/3K4/8/8 w - - 0 1", "8/3k4/3p4/8/3P4/3K4/8/8 w - - 0 1", "8/8/3k4/3p4/8/3P4/3K4/8 w - - 0 1", "k7/8/3p4/8/3P4/8/8/7K w - - 0 1",
	"8/8/7k/7p/7P/7K/8/8 b - - 0 1", "8/8/k7/p7/P7/K7/8/8 b - - 0 1", "8/8/3k4/3p4/3P4/3K4/8/8 b - - 0 1", "8/3k4/3p4/8/3P4/3K4/8/8 b - - 0 1",
	"8/8/3k4/3p4/8/3P4/3K4/8 b - - 0 1", "k7/8/3p4/8/3P4/8/8/7K b - - 0 1", "7k/3p4/8/8/3P4/8/8/K7 w - - 0 1", "7k/8/8/3p4/8/8/3P4/K7 w - - 0 1",
	"k7/8/8/7p/6P1/8/8/K7 w - - 0 1", "k7/8/7p/8/8/6P1/8/K7 w - - 0 1", "k7/8/8/6p1/7P/8/8/K7 w - - 0 1", "k7/8/6p1/8/8/7P/8/K7 w - - 0 1",
	"k7/8/8/3p4/4p3/8/8/7K w - - 0 1", "k7/8/3p4/8/8/4P3/8/7K w - - 0 1", "7k/3p4/8/8/3P4/8/8/K7 b - - 0 1", "7k/8/8/3p4/8/8/3P4/K7 b - - 0 1",
	"k7/8/8/7p/6P1/8/8/K7 b - - 0 1", "k7/8/7p/8/8/6P1/8/K7 b - - 0 1", "k7/8/8/6p1/7P/8/8/K7 b - - 0 1", "k7/8/6p1/8/8/7P/8/K7 b - - 0 1",
	"k7/8/8/3p4/4p3/8/8/7K b - - 0 1", "k7/8/3p4/8/8/4P3/8/7K b - - 0 1", "7k/8/8/p7/1P6/8/8/7K w - - 0 1", "7k/8/p7/8/8/1P6/8/7K w - - 0 1",
	"7k/8/8/1p6/P7/8/8/7K w - - 0 1", "7k/8/1p6/8/8/P7/8/7K w - - 0 1", "k7/7p/8/8/8/8/6P1/K7 w - - 0 1", "k7/6p1/8/8/8/8/7P/K7 w - - 0 1",
	"3k4/3pp3/8/8/8/8/3PP3/3K4 w - - 0 1", "7k/8/8/p7/1P6/8/8/7K b - - 0 1", "7k/8/p7/8/8/1P6/8/7K b - - 0 1", "7k/8/8/1p6/P7/8/8/7K b - - 0 1",
	"7k/8/1p6/8/8/P7/8/7K b - - 0 1", "k7/7p/8/8/8/8/6P1/K7 b - - 0 1", "k7/6p1/8/8/8/8/7P/K7 b - - 0 1", "3k4/3pp3/8/8/8/8/3PP3/3K4 b - - 0 1",
	"8/Pk6/8/8/8/8/6Kp/8 w - - 0 1", "n1n5/1Pk5/8/8/8/8/5Kp1/5N1N w - - 0 1", "8/PPPk4/8/8/8/8/4Kppp/8 w - - 0 1", "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1",
	"8/Pk6/8/8/8/8/6Kp/8 b - - 0 1", "n1n5/1Pk5/8/8/8/8/5Kp1/5N1N b - - 0 1", "8/PPPk4/8/8/8/8/4Kppp/8 b - - 0 1", "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1",
	"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", "rnbqkb1r/ppppp1pp/7n/4Pp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3",
};

static const std::vector<std::vector<uint64_t>> GIGANTUA_RESULTS
{
	{ 20, 400, 8902, 197281, 4865609, 119060324 }, { 48, 2039, 97862, 4085603, 193690690 }, { 15, 66, 1197, 7059, 133987, 764643 }, { 16, 71, 1287, 7626, 145232, 846648 },
	{ 5, 75, 459, 8290, 47635, 899442 }, { 5, 80, 493, 8897, 52710, 1001523 }, { 26, 112, 3189, 17945, 532933, 2788982 }, { 5, 130, 782, 22180, 118882, 3517770 },
	{ 12, 38, 564, 2219, 37735, 185867 }, { 15, 65, 1018, 4573, 80619, 413018 }, { 3, 32, 134, 2073, 10485, 179869 }, { 4, 49, 243, 3991, 20780, 367724 },
	{ 26, 568, 13744, 314346, 7594526, 179862938 }, { 25, 567, 14095, 328965, 8153719, 195629489 }, { 25, 548, 13502, 312835, 7736373, 184411439 }, { 25, 547, 13579, 316214, 7878456, 189224276 },
	{ 26, 583, 14252, 334705, 8198901, 198328929 }, { 25, 560, 13592, 317324, 7710115, 185959088 }, { 25, 560, 13607, 320792, 7848606, 190755813 }, { 5, 75, 459, 8290, 47635, 899442 },
	{ 5, 80, 493, 8897, 52710, 1001523 }, { 15, 66, 1197, 7059, 133987, 764643 }, { 16, 71, 1287, 7626, 145232, 846648 }, { 5, 130, 782, 22180, 118882, 3517770 },
	{ 26, 112, 3189, 17945, 532933, 2788982 }, { 3, 32, 134, 2073, 10485, 179869 }, { 4, 49, 243, 3991, 20780, 367724 }, { 12, 38, 564, 2219, 37735, 185867 },
	{ 15, 65, 1018, 4573, 80619, 413018 }, { 26, 568, 13744, 314346, 7594526, 179862938 }, { 26, 583, 14252, 334705, 8198901, 198328929 }, { 25, 560, 13592, 317324, 7710115, 185959088 },
	{ 25, 560, 13607, 320792, 7848606, 190755813 }, { 25, 567, 14095, 328965, 8153719, 195629489 }, { 25, 548, 13502, 312835, 7736373, 184411439 }, { 25, 547, 13579, 316214, 7878456, 189224276 },
	{ 14, 195, 2760, 38675, 570726, 8107539 }, { 11, 156, 1636, 20534, 223507, 2594412 }, { 19, 289, 4442, 73584, 1198299, 19870403 }, { 3, 51, 345, 5301, 38348, 588695 },
	{ 17, 54, 835, 5910, 92250, 688780 }, { 15, 193, 2816, 40039, 582642, 8503277 }, { 16, 180, 2290, 24640, 288141, 3147566 }, { 4, 68, 1118, 16199, 281190, 4405103 },
	{ 17, 54, 835, 5910, 92250, 688780 }, { 3, 51, 345, 5301, 38348, 588695 }, { 17, 278, 4607, 76778, 1320507, 22823890 }, { 21, 316, 5744, 93338, 1713368, 28861171 },
	{ 21, 144, 3242, 32955, 787524, 7881673 }, { 7, 143, 1416, 31787, 310862, 7382896 }, { 6, 106, 1829, 31151, 530585, 9250746 }, { 17, 309, 5133, 93603, 1591064, 29027891 },
	{ 7, 143, 1416, 31787, 310862, 7382896 }, { 21, 144, 3242, 32955, 787524, 7881673 }, { 19, 275, 5300, 104342, 2161211, 44956585 }, { 36, 1027, 29215, 771461, 20506480, 525169084 },
	{ 19, 275, 5300, 104342, 2161211, 44956585 }, { 36, 1027, 29227, 771368, 20521342, 524966748 }, { 2, 36, 143, 3637, 14893, 391507 }, { 2, 36, 143, 3637, 14893, 391507 },
	{ 6, 35, 495, 8349, 166741, 3370175 }, { 22, 43, 1015, 4167, 105749, 419369 }, { 2, 36, 143, 3637, 14893, 391507 }, { 6, 35, 495, 8349, 166741, 3370175 },
	{ 3, 7, 43, 199, 1347, 6249 }, { 3, 7, 43, 199, 1347, 6249 }, { 1, 3, 12, 80, 342, 2343 }, { 1, 3, 12, 80, 342, 2343 },
	{ 7, 35, 210, 1091, 7028, 34834 }, { 1, 3, 12, 80, 342, 2343 }, { 1, 3, 12, 80, 342, 2343 }, { 3, 7, 43, 199, 1347, 6249 },
	{ 3, 7, 43, 199, 1347, 6249 }, { 5, 35, 182, 1091, 5408, 34822 }, { 2, 8, 44, 282, 1814, 11848 }, { 2, 8, 44, 282, 1814, 11848 },
	{ 3, 9, 57, 360, 1969, 10724 }, { 3, 9, 57, 360, 1969, 10724 }, { 5, 25, 180, 1294, 8296, 53138 }, { 8, 61, 483, 3213, 23599, 157093 },
	{ 8, 61, 411, 3213, 21637, 158065 }, { 4, 15, 90, 534, 3450, 20960 }, { 3, 9, 57, 360, 1969, 10724 }, { 3, 9, 57, 360, 1969, 10724 },
	{ 5, 25, 180, 1294, 8296, 53138 }, { 8, 61, 411, 3213, 21637, 158065 }, { 8, 61, 483, 3213, 23599, 157093 }, { 4, 15, 89, 537, 3309, 21104 },
	{ 4, 19, 117, 720, 4661, 32191 }, { 5, 19, 116, 716, 4786, 30980 }, { 5, 22, 139, 877, 6112, 41874 }, { 4, 16, 101, 637, 4354, 29679 },
	{ 5, 22, 139, 877, 6112, 41874 }, { 4, 16, 101, 637, 4354, 29679 }, { 3, 15, 84, 573, 3013, 22886 }, { 4, 16, 101, 637, 4271, 28662 },
	{ 5, 19, 117, 720, 5014, 32167 }, { 4, 19, 117, 712, 4658, 30749 }, { 5, 22, 139, 877, 6112, 41874 }, { 4, 16, 101, 637, 4354, 29679 },
	{ 5, 22, 139, 877, 6112, 41874 }, { 4, 16, 101, 637, 4354, 29679 }, { 5, 15, 102, 569, 4337, 22579 }, { 4, 16, 101, 637, 4271, 28662 },
	{ 5, 22, 139, 877, 6112, 41874 }, { 4, 16, 101, 637, 4354, 29679 }, { 5, 22, 139, 877, 6112, 41874 }, { 4, 16, 101, 637, 4354, 29679 },
	{ 5, 25, 161, 1035, 7574, 55338 }, { 5, 25, 161, 1035, 7574, 55338 }, { 7, 49, 378, 2902, 24122, 199002 }, { 5, 22, 139, 877, 6112, 41874 },
	{ 4, 16, 101, 637, 4354, 29679 }, { 5, 22, 139, 877, 6112, 41874 }, { 4, 16, 101, 637, 4354, 29679 }, { 5, 25, 161, 1035, 7574, 55338 },
	{ 5, 25, 161, 1035, 7574, 55338 }, { 7, 49, 378, 2902, 24122, 199002 }, { 11, 97, 887, 8048, 90606, 1030499 }, { 24, 421, 7421, 124608, 2193768, 37665329 },
	{ 18, 270, 4699, 79355, 1533145, 28859283 }, { 24, 496, 9483, 182838, 3605103, 71179139 }, { 11, 97, 887, 8048, 90606, 1030499 }, { 24, 421, 7421, 124608, 2193768, 37665329 },
	{ 18, 270, 4699, 79355, 1533145, 28859283 }, { 24, 496, 9483, 182838, 3605103, 71179139 }, { 14, 191, 2812, 43238, 674624, 11030083 }, { 31, 570, 17546, 351806, 11139762, 244063299 },
};

// these positions were chosen randomly from lichess's elite database from 2021 October
static constexpr std::string_view GAME_FENS[128]
{
	"8/8/8/5kp1/r7/3R1PK1/6P1/8 w - - 9 71", "6k1/5pp1/1pPqp2p/8/p7/2R3PP/5PBK/8 b - - 2 33", "5nk1/r2b2pp/p1nbp3/1p2P3/5P1P/P7/BPP5/1K1R3R w - - 0 24", "3n4/1p5B/1k2pR2/1b1pP3/1P6/6P1/5P2/b5K1 w - - 3 33",
	"8/8/8/p7/1q6/1P6/2K4k/8 b - - 3 67", "r4rk1/2p1q1b1/1p6/p1n1ppp1/2P5/1P2PNP1/PBQ2PK1/R6R b - - 3 22", "1kbr3r/pp2qpp1/1np2n1p/N1N5/3R2P1/4P2P/PPQ1BP2/1K1R4 b - - 0 21", "2k5/1p2R3/pP1p4/P1pP1b2/2r3p1/2P4p/4RP1K/8 b - - 3 42",
	"r2qnrk1/pp3bpp/2pp1b2/8/P3P2P/2N2P2/1nP1NBB1/R2QK2R w KQ - 0 18", "4rnk1/5pp1/2Nb1p1p/7b/2B1q3/4B2P/1P3PP1/R1R3K1 w - - 4 26", "1r3r1k/p1p1q1bp/3pNpp1/3B4/2P5/1P4P1/PB1QPP2/5RK1 b - - 10 24", "3r1rk1/p4pb1/bpn1pqpp/2p5/PPN1R1P1/3P1NBP/1PQ2PB1/4R1K1 b - - 0 20",
	"r2q1rk1/1b1n1pbp/3pP1p1/p1nN4/1p1NP3/3BB2P/PPP1Q1P1/3R1RK1 b - - 0 16", "8/8/7R/5kp1/r7/5PK1/6P1/8 b - - 16 74", "2r2bk1/1p3q1p/6p1/N3npB1/P3p2Q/8/2P2PPP/3R2K1 w - - 0 28", "2q2k2/5pp1/1RP1p2p/1p6/p7/6PP/5PBK/8 b - - 5 37",
	"5n2/2rb1kpp/pR2p3/1p2Pn1P/2P2P2/P7/BP6/1K5R w - - 1 28", "3n4/1p6/1k2pRb1/3pP1P1/1b3P2/8/8/6K1 w - - 0 37", "r1b2rk1/pppnq1b1/2np2pp/4p3/2BP1p2/P1P1PN1P/1P1N1PPB/R2Q1RK1 b - - 3 13", "3r1rk1/2p3b1/1pq5/p1n1pR2/2P3pN/1P2P1P1/PBQ2P2/R5K1 b - - 2 26",
	"2b5/ppk1qpp1/1np2n1p/N1N5/5QP1/4P2P/PP2BP2/1K6 b - - 2 25", "8/1p1k3R/pP1p4/PbpP4/2r5/2P3Pp/4R2K/8 b - - 4 46", "r2qnrk1/BQ4pp/2pp4/P3n2b/4P2b/2N2P2/2P1N1B1/R3K2R w KQ - 0 22", "4r1k1/R4pp1/2Nb1pqp/7b/2R5/4B2P/1P3P2/5BK1 w - - 1 30",
	"1r5k/p3q1b1/3p2p1/3P1p2/8/1P4P1/PB1QPP2/2R3K1 b - - 1 28", "3r1rk1/p3qpB1/bp2p1pp/2p5/PnN1R1P1/4RN1P/1P3PB1/1Q4K1 b - - 0 24", "r1b2k2/ppp2pp1/4p2r/3q2N1/3P1Q2/8/PPP2P1P/2KR2R1 b - - 5 20", "2r2rk1/2qp2pp/1b2p1n1/p2nPp2/Pp3P2/1P1N2PP/R1PB2K1/3Q1RN1 b - - 6 22",
	"r4rk1/ppp2ppp/2np1q2/6b1/2BPPnQ1/2N1B3/PP3P1P/R3R1K1 b - - 3 14", "5k2/5p2/2pRr3/2P1P2p/5K1P/8/8/8 b - - 2 50", "r5k1/p7/2p3pP/2p1Rp2/6nQ/3q4/P7/4R2K b - - 1 32", "8/5ppk/1pQb3p/1Pp2q2/2Pp1P2/6R1/r2B4/5NK1 b - - 5 37",
	"1qr2rk1/p2nbppp/1p2p3/2pb4/3PP3/1P4P1/PB1N1PBP/2RQR1K1 b - - 0 16", "r1b1q1kr/1p1pb3/p1n1p1p1/4PnN1/4QBpP/2P2P2/PP1N4/2KR3R w - - 0 17", "r1b1rn1k/pp2q3/2pb1p2/7Q/3P4/2P1B3/PP2NPPP/2KR3R b - - 6 15", "8/7p/5kpP/R7/p4P2/P7/8/K7 w - - 3 49",
	"8/P2R2p1/7p/5p2/7P/r3kP2/3p2PK/8 w - - 1 43", "4k3/p3R3/1p3P2/6PK/2p5/3p4/5r2/8 b - - 3 49", "5rk1/4np1P/2p5/3p4/8/1p2N2P/rP3qB1/1R3R1K b - - 0 30", "3rr1k1/1p3pbp/1qppb1p1/2P5/1P1BP3/2Q4P/4B1P1/1R1R2K1 b - - 0 24",
	"8/4K3/7p/6rk/R7/8/8/8 w - - 16 62", "r1bq2k1/2pp1r1p/pbn5/1p1Q1p2/3Pp3/P1PBBN1P/1P4P1/R4RK1 w - - 0 18", "3r2k1/p1p2ppp/2p5/8/3NNP1b/1PP1B1rb/P7/4R2K b - - 0 23", "4rr2/6bk/p1pp3p/2p1pp2/2P2P2/P1P3P1/1P5P/R1B2RK1 b - - 0 21",
	"8/8/pp1p2p1/2bP1k1p/P7/6P1/5PK1/4R3 w - - 0 48", "2kr2r1/pp2bp1p/2p1p3/3P4/2P1P3/2NQB3/PP1R2P1/1K5q w - - 1 23", "3r4/pb2bkpp/1p2pp2/3n4/3N4/1BP3BP/PP3PP1/3R2K1 w - - 4 24", "r4rk1/pbpnq1pp/1p1pp3/5p2/1QPPnB2/P3PN2/1P2BPPP/R4RK1 b - - 6 13",
	"5r1r/2qkb1p1/Q1p1P3/P2n4/3B1pn1/5NP1/5PB1/2R1R1K1 b - - 0 29", "r1b2rk1/pp3ppp/1qn5/2b1p3/8/1BP1BQ2/PP2NPPP/R4RK1 w - - 2 14", "1R6/3nkprp/8/4q3/8/Q7/2P2PPP/6K1 b - - 4 27", "2r1r1k1/1b2pp2/p2p1b1p/1p1P1p2/1P6/2P1BN1P/1P3PP1/3RR1K1 w - - 2 22",
	"7k/p7/7p/2P5/4RR2/2P5/r6r/5K2 w - - 0 35", "8/6k1/p7/3P1p1R/3K1P2/1P4Pp/7r/8 b - - 3 41", "5rk1/pp2r1bp/3p2p1/5p1q/2NP1P2/1P2P3/P4Q1P/R4R1K b - - 0 22", "r1b1r2k/pp1nq3/2pb1pQ1/8/3P4/2P1B3/PP2NPPP/2KR3R b - - 4 14",
	"8/3k4/1Kpb1B2/7p/p6P/8/8/8 b - - 1 53", "8/p2k2b1/2pPp3/5p1p/2P5/1P6/1P4pP/2K1R3 w - - 0 25", "8/p4k2/2p2pp1/2N4p/PPn1PK1P/6P1/3r4/2R5 b - - 3 46", "8/8/5p2/3k2p1/8/4N1PP/r4PK1/8 b - - 10 67",
	"5rk1/2q2pp1/1pp1pb2/p7/2PQ3p/1P4P1/PB2PP1P/R2R2K1 w - - 0 17", "r1b2rk1/pp3ppp/1np1q3/4P3/1bP2B2/2N4P/P2NQPP1/R1R3K1 b - - 5 19", "1k5r/1pp1qpb1/p3p2p/4P1p1/1n3P2/2N1Q1P1/PPP1N1P1/2KR4 w - - 2 21", "8/1kp5/1ppb3p/7P/p2P4/P1P1B3/1PQK1P2/7q w - - 13 44",
	"4rq2/ppbn2kp/2p1n1p1/2Pp1p2/1P1P4/4NNP1/P1QB1P1P/4R2K w - - 1 23", "8/pp2q1k1/1nn3pp/4P3/5QP1/1PP4P/P5K1/4R3 w - - 3 31", "R7/8/3p1pk1/b7/1p6/4P3/4KP2/1r2B3 b - - 6 52", "r5k1/2pq2bp/3p1np1/p3pb2/3nN3/PP1P2P1/1BPN1P1P/R2QK2R w KQ - 1 15",
	"8/p7/P2r1p1p/5n2/5kP1/5R2/5K2/3B4 b - - 1 68", "r3r1k1/pp1b2pp/1qn1pb2/3p4/3PnN2/P2Q1N1P/1PB2PP1/R1B1R1K1 w - - 3 18", "r2rb1k1/pp2qp1p/2n1p3/5p2/1b5N/1QN3P1/P1R1PPBP/2R3K1 b - - 7 19", "5rk1/pp3ppp/2q2n2/4p3/2PP4/P4P2/1B1P1P1P/1Q3RK1 b - - 0 19",
	"8/8/5bN1/5P2/4k1K1/8/8/8 w - - 4 71", "6k1/8/4pp2/2Np3p/3P4/4P2P/2r3P1/5K2 b - - 1 30", "8/8/3b4/5kp1/1p3P2/5KBp/8/8 w - - 0 58", "N4k1r/pp2pp2/2n3p1/7p/Q3n2P/8/3B1Pq1/2R1K2R w K - 0 23",
	"5rk1/p2R2bp/6p1/5p1r/2Q1p2q/2N1B2P/PP3PP1/5RK1 b - - 1 24", "3R4/8/8/8/7P/2k5/r7/6K1 w - - 8 70", "6k1/4R3/p6p/1p3Pp1/3B1p2/P6P/5K2/2r5 b - - 2 39", "8/1p2p1k1/3r2p1/4p3/1P1bP1P1/PKN2P1R/8/8 w - - 5 37",
	"8/5k2/R7/5Np1/4P1P1/3r1PKP/8/8 b - - 0 57", "8/3N1kp1/4pp1p/8/K6P/P5P1/5n2/8 w - - 2 40", "2r1rbk1/5pp1/p3b2p/3p3q/1P1Nn3/P3P1P1/Q4P2/B1RR2KB b - - 6 32", "8/1p3r2/p2R3p/6pk/6r1/2P5/PP2RP1P/5K2 b - - 9 35",
	"rr4k1/p2b1ppp/2p5/2Pp4/1P1N4/P3P3/3K1PPP/2R4R w - - 1 19", "1k6/8/2R5/2K5/P1P5/8/1P6/8 b - - 2 63", "rn1q1rk1/1b2bpp1/2p4p/2pp2P1/1p1P3P/3BPN2/P1Q2P2/R3K2R w KQ - 1 16", "4rr2/2qn3p/1pPb3k/p4p1p/P1P5/3NP2Q/1B4P1/7K b - - 0 28",
	"8/p2k1pp1/1p4p1/3pP3/4K3/1P5P/P2bN1P1/8 w - - 0 38", "r3k3/1p3pp1/p4b2/3p4/3P1r2/P1N4b/1PP1BBnP/R2K3R w q - 6 25", "5r1k/2r2pp1/p6p/1p6/7n/1B1R1P2/PPP2P1P/4R1K1 b - - 2 27", "2r3k1/5ppp/1Pb5/p7/5P2/8/P5PP/2Q2B1K b - - 0 31",
	"5r1k/p5bn/3p1qQ1/3B4/2P5/6P1/P4P1P/5RK1 w - - 7 31", "r3rn2/p1p3kp/1p3R2/4P2q/1PP5/P1Q3P1/1B6/4R1K1 w - - 1 28", "6r1/1k2b3/4p3/1P1pB3/4qP2/2P3rp/3Q2PK/3R2R1 w - - 0 38", "8/7p/1p4pk/1P3p2/2PQn3/4R2P/2q3PK/8 b - - 2 41",
	"1r3k2/5b1p/p4Q1N/Pp3R2/2p5/8/1PP3PP/3q2K1 w - - 1 32", "8/5k2/1R6/p1b2B2/P1P5/7P/2K5/5r2 w - - 3 55", "r1bqr1k1/4bpp1/p2p1n1p/np2p3/2p1P3/P1PP1NN1/1PB2PPP/R1BQ1RK1 w - - 0 14", "8/5p1p/4p1p1/2r5/2P2k2/1R1K1P1P/6P1/8 w - - 1 41",
	"6Q1/5K2/4p3/n7/1p2kP2/8/8/8 b - - 0 56", "rn5r/pp1kb1p1/2p1p2n/3pP1NP/6P1/8/PPP5/RNB2RK1 w - - 3 18", "8/7p/P1k5/1nNpK3/3P4/8/7P/8 b - - 2 43", "4n3/2p2p2/1p1p2p1/p2P3p/2P3PP/1Pk2B2/P3KP2/8 b - - 0 36",
	"8/8/5pkp/p2R2p1/Pr6/1P3P2/4KP1P/8 w - - 6 37", "2r4r/1b4k1/p3p1Qp/3p4/3P3P/2P5/P1B2PP1/6K1 b - - 0 33", "k3r3/1p3p2/2b5/p6p/2P2P1P/3B3R/2P3P1/4Q1K1 b - - 0 33", "8/5B2/3b4/8/1p6/1P3Kp1/3k2P1/8 b - - 13 49",
	"8/8/4kpp1/1p4Pp/p4K1P/Pr3P2/1P1R4/8 w - - 2 49", "r5k1/1p4pp/p1p1rpn1/3q4/8/1P1PB1PP/P1P2Q1K/4RR2 w - - 1 25", "r1b2rk1/p1q2ppp/2p5/1pb1pN2/4P1n1/2P2NPP/PPB1Q1K1/R3R3 b - - 0 20", "5rk1/pppqnrbn/3p2pp/3Ppb2/2P1N2P/1P3PP1/PB4BK/2Q1RRN1 b - - 2 21",
	"3R4/5pk1/5np1/4K3/4P3/2P5/2P3P1/8 b - - 0 36", "8/7P/5K2/1p1k4/1P6/5p2/7q/8 w - - 1 50", "8/4k2r/1R6/R3P3/8/2P5/2K5/8 b - - 12 49", "r4rk1/p1p1qppp/2p3b1/3pP3/8/2PBP3/PPQn2PP/R4RK1 w - - 0 14",
	"3r1rk1/pRp3pn/8/3P4/6P1/4BP2/q5P1/3QR1K1 w - - 1 23", "8/7p/5kpP/3K4/2p5/2P5/8/8 b - - 1 55", "8/8/4k1p1/6B1/2K2R1P/8/4r3/8 b - - 6 59", "2rr2k1/5pp1/p1b2q2/1p2B2p/8/3B4/PP2QPPP/2R2RK1 b - - 0 26",
	"1r1q1r2/pp4kp/4bpp1/2Qnp3/4BP2/3P2P1/PP2N2P/R1R3K1 w - - 3 20", "5k2/8/1p6/p1bPKnP1/7r/8/P7/8 w - - 0 56", "r5k1/1R3pp1/3p4/2p4p/5P2/P2K2P1/7P/2B3b1 w - - 2 29", "1rb1r1k1/1p2qpb1/p2p1nnp/2pP2p1/P3P3/R1N3PP/1P1N1P1K/2BQRB2 w - - 1 18",
	"1q6/5pk1/4pQp1/p2pP3/1n1P4/3b2PB/P4P2/6K1 b - - 1 35", "1brr2k1/1b2qppp/1p2pn2/p2p4/1P1Nn3/P1PBPN1P/1B2QPP1/2RR2K1 b - - 6 17", "8/B7/4k1p1/5b2/1P2K3/8/P7/8 w - - 1 47", "5b2/5qk1/6p1/8/6K1/2Q5/8/8 b - - 9 54",
};

static constexpr uint64_t GAME_RESULTS[128]
{
	66809, 323393, 550883, 161455, 20398, 3182415, 5323344, 145652, 2160677, 4864018, 2144888, 3726314, 4620594, 62183, 2547325, 117423,
	413493, 73351, 326336, 2590549, 145768, 17943, 469591, 230117, 1947441, 2803109, 2236235, 1564761, 3864105, 32988, 1313541, 1794700,
	2240886, 2577345, 260481, 5394, 201357, 11167, 55907, 2702022, 76447, 2486647, 1132492, 162150, 71488, 348966, 1462445, 2969417,
	246877, 2497799, 234326, 802433, 271738, 43190, 1538224, 1883455, 38729, 116657, 314613, 23618, 1588058, 2357550, 1213146, 389225,
	2661935, 1617721, 124201, 2124667, 27058, 4214430, 2835441, 908754, 18392, 50409, 17914, 1935643, 175705, 80923, 263983, 284043,
	122464, 30125, 2929608, 540084, 702427, 3109, 1685438, 1029523, 26397, 972041, 867126, 440192, 1183319, 1522410, 2299945, 592396,
	37794, 360137, 1101708, 107581, 74568, 832747, 19783, 40328, 136619, 15519, 735529, 18145, 29102, 2324954, 2818315, 1667430,
	32833, 54973, 186466, 2246694, 2081065, 1383, 81470, 3003084, 3474180, 45979, 302873, 1794755, 60223, 3153389, 10340, 56808,
};

// these positions are generated by performing a random number of random moves from the starting position
// they're pretty entertaining to look at
// they contain a myriad of edge cases (did you know that myriad is technically the number 10'000)
static constexpr std::string_view RAND_FENS[128]
{
	"8/3n2N1/5P1k/1R6/2K4P/8/8/8 w - - 1 122", "rn3b2/5r2/1n2k3/pQ1pp2p/1p1P4/B1P3PP/P4qK1/RN3R2 w - - 3 48", "3rk3/8/3n4/2K5/7p/3p3N/2b5/8 w - - 7 107", "8/8/5k2/8/5K2/8/8/8 w - - 35 156",
	"1r5B/1p6/1PpkPnN1/6qp/1pB1P3/1n4R1/P5K1/8 b - - 3 43", "1Q6/4k3/8/8/3R4/3B4/8/K7 w - - 5 157", "8/7K/8/3r4/8/8/7R/2k5 w - - 60 200", "8/8/8/b1K1k3/3r1p2/8/8/8 b - - 3 136",
	"2k5/p7/P7/1p6/1Pb5/3K4/8/8 w - - 31 146", "8/4Rr1p/1k6/8/1p3p2/8/4N1KB/1b2q3 w - - 4 79", "r1k5/2P1n3/2R1P3/pr6/1p6/5p2/1P1b4/3K4 w - - 6 69", "8/3n4/2k5/p7/P1p5/1R6/7B/2K5 w - - 14 109",
	"8/8/3K4/8/7p/5k2/8/8 w - - 1 171", "6k1/K7/5Q2/6P1/7p/B7/8/1b6 w - - 11 115", "2b1k1nr/p7/1p3p2/3p3p/qb5P/P2P1KP1/2PB1NR1/6R1 b - - 1 38", "rq1k2n1/p1p3pr/1p3pQp/4b3/2P5/1P1PP1P1/3R1P2/1N2KBNR w K - 1 22",
	"1k6/5r2/p4bp1/pB6/3Pb3/3r3p/2R4P/K7 w - - 0 79", "3k4/8/3r4/3P1Pp1/PP4P1/1q1B1K2/3R1n2/8 b - - 0 64", "5R2/8/1PP4R/P7/8/8/7K/3k4 b - - 24 132", "5kn1/8/2p5/p1p5/P1P5/Bp2p3/5N2/2K5 w - - 16 82",
	"8/2P1k3/3N2n1/8/4K2p/8/8/3B4 b - - 8 121", "8/1B1k3p/1P4rP/5N2/P4ppP/1K2R3/2R5/8 b - - 2 78", "6N1/8/1Pk4P/2p5/r1p3r1/P1P2p2/2p1NKRB/7b b - - 3 94", "1Q1qk3/2pbpn2/n1rp2r1/P4pPp/2PbP1PP/N3R3/3B4/4KBNR w K - 5 25",
	"8/8/4p2k/5P2/8/rp3Rpp/4K3/5r2 b - - 5 84", "r1brB3/8/3k3p/pK6/2P2RpP/5p2/4q3/8 b - - 12 61", "4kr2/p3P3/1pr2p2/n1p4p/B2p2Np/P4PbP/RB6/Q4KNR b - - 5 44", "4K3/8/8/8/2k5/8/8/8 b - - 8 185",
	"5bnB/4q3/1k6/pPpnQ2P/2p1PNpp/N4P2/P5rP/3RK2R b - - 0 38", "3k4/8/8/P5K1/8/8/7n/5B2 b - - 16 142", "2Br4/4b3/1kp5/ppR5/1P3pP1/PK6/6N1/B7 b - - 3 73", "7N/3K4/8/8/8/8/3k4/8 b - - 26 190",
	"8/7P/2P2k2/7B/p3p3/8/8/K7 b - - 6 83", "8/5RB1/8/8/P7/4R3/8/2K4k b - - 124 186", "8/2r5/8/5b1N/P7/5k2/8/1b5K b - - 24 155", "4R3/3k4/P6P/2P5/3pNpqP/p7/K3R3/8 b - - 4 73",
	"3B1k2/8/5K2/8/8/8/8/8 w - - 13 192", "8/4K1R1/1k6/6p1/2P2rPn/8/3r3N/5B2 w - - 5 71", "r3kr2/p2n4/1p5b/1Qpp1P1p/P1n1p2p/3PPP1P/4K1B1/RN4NR b q - 0 25", "8/8/5k2/8/8/3K1p2/3B1r2/8 w - - 12 122",
	"5rQr/pppnk1p1/q3b3/2p1p3/4pPPb/1P1PB3/P3R2P/1N1K2NR w - - 1 21", "2k2r2/1n6/Rb6/1p1P2p1/8/P5B1/4K3/8 b - - 9 88", "8/8/2b2p2/Pk3P1p/7P/5K2/2B5/b7 w - - 0 85", "b4k2/1q1n1p2/1p1Pp2n/2P2PbP/2pK2P1/2P3N1/4B3/RRN5 b - - 10 48",
	"6R1/5k1p/1P6/1P6/r1N1Pn2/PP5R/K7/4bn2 w - - 0 55", "8/6n1/2P5/rp2N1p1/1P4k1/2R2N1p/7P/RK6 b - - 12 91", "N2k2n1/p2p4/4r3/nPb2P1p/3p4/P1p2P1N/5R1P/2BK3R w - - 6 39", "8/b7/3k4/n4p1K/3r4/8/6B1/8 b - - 39 130",
	"4kb1r/4p2n/p1b1P3/7r/3B3p/PPpPNN2/4R3/4K2R b - - 1 40", "4k3/P7/8/8/1K6/2NbB1q1/8/8 b - - 2 129", "3N1bnr/p1p1r1p1/8/1p1p1k1p/3P3B/2P2p1b/1PN1nPPR/2RK1B2 b - - 1 29", "2K5/8/8/8/7k/8/2b5/3n4 w - - 26 132",
	"8/8/2k5/8/8/5K2/8/8 w - - 53 181", "3k4/8/1P6/2B1K3/8/8/3B4/8 b - - 12 148", "2r1n2r/3p1p2/pp1k3P/8/nPP3PB/1N6/3KP3/4RBNR w - - 0 46", "3k4/5R2/1n2P3/p2NP3/8/8/3B4/3K2n1 w - - 3 87",
	"8/7K/4k3/8/8/2NP4/8/8 b - - 8 114", "8/8/6K1/1k6/8/8/8/8 b - - 143 185", "8/8/3p2k1/3P4/8/5K2/8/7q w - - 0 110", "8/8/8/8/8/6K1/k7/8 w - - 48 194",
	"8/8/8/p2N4/k4K2/1r6/8/8 b - - 9 137", "1n3r2/6kp/2p2p2/2P5/P4PPb/1R6/b3K2N/8 b - - 2 51", "8/8/8/2r1K3/p7/8/3k4/8 w - - 36 122", "3q3r/n2p1r2/bp2kb2/1B5p/P3Pp1P/R1B2P1N/4K3/1Q5R w - - 27 60",
	"3r4/5nk1/6p1/pP2p2p/4P2P/1p4K1/6p1/8 b - - 7 67", "rn5n/p1r4p/3P1kb1/Pp5P/2NP1p2/5P2/8/2KR4 b - - 0 57", "r4b1r/pbk1n1p1/2p1pp2/1p4Bp/nPpqPN1P/N1P2PP1/P4K2/2R2B1R w - - 0 19", "8/8/5K2/8/8/5k2/8/8 b - - 90 174",
	"8/8/P7/2N3k1/8/1K6/8/6q1 b - - 24 156", "8/8/8/3K4/8/8/4k3/8 w - - 12 176", "8/8/7K/4k3/8/8/8/8 b - - 25 174", "8/4p1B1/pk2Pnbn/8/BP1P1pP1/R1P5/4q3/1NKQ4 b - - 18 59",
	"8/8/8/6Pp/4k3/1n5K/1n6/8 w - - 3 161", "2B3n1/4b3/r7/P2nB2p/2N2P1k/3R4/7K/1r6 b - - 1 87", "8/8/3k3K/8/8/8/8/8 w - - 30 185", "8/2n1k3/r7/N7/8/4p3/8/1K3R2 w - - 2 105",
	"2r5/5n2/2R5/1PNn2p1/1PB3k1/8/8/2K3B1 b - - 15 105", "8/8/4N2k/8/1r6/r7/6K1/8 w - - 28 148", "8/4k3/8/p1P5/B7/p5pP/3K4/1r4b1 w - - 3 98", "2r3rk/2p4p/npq2p1P/2b2b2/3p2PQ/pPN1p1R1/PBP5/1R2K1N1 b - - 5 36",
	"8/7k/7N/1r1b1P2/3K4/2P5/8/8 b - - 8 94", "kr6/6bQ/3p3R/p6p/2nP1N1P/1p6/3N4/3K3R w - - 3 56", "8/8/1r6/8/4BK2/7k/8/8 w - - 14 95", "r1b1k1nr/pppp3p/5pp1/P3p1P1/1bPnN2P/3P4/1PQ1PKB1/6NR w kq - 3 18",
	"rnb1k1r1/n2p1p2/1P6/p1pqpp1p/2R1P1QP/1PNBb1P1/2PP4/2B2KNR w q - 1 24", "1n6/6rk/p1p3p1/P2p4/R2P3b/K3R1Pb/8/8 w - - 0 62", "8/8/k7/8/6n1/2N3K1/1R6/8 b - - 34 170", "4rQR1/3k3b/8/5p2/p6N/B2B1N2/3K4/8 w - - 10 78",
	"2b5/3k1n1r/n2b4/r3PP2/p1pP3p/p7/P7/K6R b - - 1 62", "8/8/1pr2k1n/p1p1pN1p/P2nPp1b/1R6/1B4K1/5R2 b - - 5 52", "3K3B/3r3R/8/8/2k5/8/6B1/8 w - - 24 154", "1b6/4k3/1B6/8/1P6/8/1K6/6N1 w - - 17 85",
	"1r4n1/p2kq3/bpn5/P1pKpppr/1b1p1Q1P/3PPPP1/R5R1/6NB w - - 0 39", "8/6R1/8/7p/7P/5k2/8/2K5 w - - 6 167", "1rk5/4r3/2P1p2p/2P5/pP1p3p/3PP1p1/8/K3RBb1 b - - 0 53", "2r5/8/P4R2/8/8/3K2k1/8/8 w - - 19 151",
	"5N2/8/8/K7/5k2/8/8/8 w - - 85 186", "8/2r1k3/7P/1p5P/8/8/3K4/8 w - - 1 141", "r1r5/k2nQ1bp/5p2/p1P3p1/b4P2/4PB1P/N1P3Pq/3K3R w - - 15 36", "8/2k1K3/8/1pP2p2/2p5/RRP5/6b1/8 b - - 20 132",
	"8/B7/P2k4/8/8/5K2/8/8 w - - 35 132", "2k5/4b3/8/P7/2p5/8/2p4B/4K3 w - - 0 172", "8/6b1/8/3k4/K3B3/8/8/8 b - - 95 190", "7B/8/8/b7/K7/3r1k2/8/8 w - - 7 187",
	"2k5/8/8/5p2/8/K4nN1/8/2r5 w - - 36 155", "8/8/1k5p/5p1P/R2p1P2/pN1K3N/P7/8 w - - 2 83", "2K3k1/N7/1P1b4/7p/p6P/8/3q4/8 b - - 8 90", "7Q/8/8/b7/4k3/8/8/6K1 b - - 7 149",
	"Nn6/6k1/4n3/1Bp3p1/2P1p3/1PK1P1r1/8/8 b - - 3 50", "6rQ/1q1k4/rp6/p1b1pNp1/Pnp2P2/2R1p1P1/1P2K3/R4B2 b - - 4 43", "3n4/q2r4/3rp2b/1b3Ppk/3PP1pP/8/4N3/4K3 w - - 3 67", "8/8/4r2k/8/1p3R2/8/3K4/8 w - - 1 104",
	"r3kbnr/1p3pp1/2pp3p/p1N1pn2/4PP2/1P2Q1qN/PBPP3P/R2K3R w kq - 2 20", "r3kbnr/p5p1/BppP4/1N4Bp/4p3/1P1b1P1P/P1PK3P/3R2R1 b k - 1 24", "1B5k/8/1r6/6P1/8/2P5/2K5/7n w - - 4 106", "2q3k1/N1B4r/3P1Pp1/7p/2P1p2n/1p2P2P/4KR2/r7 b - - 0 65",
	"1r1k1bnr/p7/q1b3pp/2PpP3/2P1N1p1/B2PP2N/P1n4P/R4K1R b - - 0 21", "2b4k/1N6/1P2P1q1/3p1Bp1/1b1PR2p/r2P3P/5K2/8 w - - 1 57", "8/1b6/4k3/5R1p/3p2p1/5nP1/2R5/2K5 w - - 4 97", "2R3N1/6k1/3P4/8/4n2p/8/8/4K3 b - - 0 99",
	"2k5/5R2/8/8/8/2K5/8/8 w - - 3 154", "8/8/8/3K4/P7/5N2/8/1k6 b - - 44 143", "8/3k2N1/8/2B5/8/2p4r/6K1/8 b - - 2 127", "8/8/8/5k2/4N3/B7/8/K1n5 w - - 11 200",
	"8/1R3P2/7R/8/8/8/4k3/6K1 b - - 10 123", "8/5k2/8/K7/8/3q4/8/8 w - - 20 193", "8/8/5P2/p6r/P7/8/1K6/6k1 w - - 12 99", "8/8/8/8/4N3/2n4k/5K2/8 w - - 0 169",
};

static constexpr uint64_t RAND_RESULTS[128]
{
	32743, 92853, 37252, 1920, 1091957, 22276, 60817, 5634, 6176, 354477, 87692, 61488, 4472, 97169, 981071, 646601,
	599231, 273500, 26221, 13919, 51011, 261630, 377861, 2421040, 104772, 221302, 397027, 2022, 1945771, 12783, 424143, 5380,
	14223, 15818, 55673, 277298, 1307, 165181, 578745, 35055, 2144231, 311013, 19965, 661226, 956775, 31773, 772518, 90110,
	917484, 577003, 676714, 10349, 3852, 6992, 547143, 119383, 6846, 2980, 15080, 1595, 35396, 253013, 12477, 1850589,
	27703, 221482, 177758, 3234, 72024, 2759, 1490, 1222117, 9402, 1664200, 1916, 174480, 812941, 124142, 98384, 1617277,
	48545, 816866, 55295, 902295, 1740336, 217079, 41849, 468502, 186466, 761518, 17218, 63546, 1257880, 11437, 124335, 103795,
	5178, 15855, 1234609, 75530, 9708, 39777, 18733, 61141, 71084, 10597, 59069, 77378, 63827, 2176799, 231504, 75290,
	2052091, 737891, 51653, 439874, 1444395, 417994, 221175, 58647, 6207, 6730, 79827, 23999, 36960, 12648, 17767, 11412,
};


static void gigantua_test()
{
	Engine juicer;

	for (size_t i = 0; i < 128; ++i)
	{
		juicer.seed(GIGANTUA_FENS[i]);
		int depth = 0;
		for (uint64_t res: GIGANTUA_RESULTS[i])
		{
			++depth;
			mu_assert(juicer.perft<false>(depth) == res, std::string(GIGANTUA_FENS[i]) + std::to_string(depth));
		}
	}
}

static void lichess_elite_test()
{
	Engine juicer;

	for (size_t i = 0; i < 128; ++i)
	{
		juicer.seed(GAME_FENS[i]);
		mu_assert(juicer.perft<false>(4) == GAME_RESULTS[i], GAME_FENS[i]);
	}
}

static void rand_test()
{
	Engine juicer;

	for (size_t i = 0; i < 128; ++i)
	{
		juicer.seed(RAND_FENS[i]);
		mu_assert(juicer.perft<false>(4) == RAND_RESULTS[i], RAND_FENS[i]);
	}

}


int main()
{
	mu_run(gigantua_test);
	mu_run(lichess_elite_test);
	mu_run(rand_test);

	return 0;
}
