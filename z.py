import chess

fens = [
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
]

def perft(board, depth):
    move_count = 0
    def perft_worker(b, d):
        nonlocal move_count
        if d == 0:
            return 1
        count = 0
        for move in b.legal_moves:
            b.push(move)
            count += perft_worker(b, d - 1)
            b.pop()
        return count

    return perft_worker(board.copy(), depth)

for fen in fens:
	board = chess.Board(fen)
	print(perft(board, 4))
