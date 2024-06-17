
typedef struct Magic
{
	uint64_t magic;
	uint64_t mask;
	uint64_t* attacks;
	uint32_t shift;
	uint32_t table_size;

	uint32_t index(uint64_t occupied) const { return (occupied & mask) * magic >> shift; }
} Magic;


Magic ROOK_MAGICS[64] = {
	{ .magic = 0x2080019880214000ull, .mask = 0x101010101017eull, .attacks = nullptr, .shift = 52, .table_size = 4096 }, // A1
	{ .magic = 0x40004020001000ull, .mask = 0x202020202027cull, .attacks = nullptr, .shift = 53, .table_size = 2048 }, // B1
	{ .magic = 0x81000830c1200300ull, .mask = 0x404040404047aull, .attacks = nullptr, .shift = 53, .table_size = 2048 }, // C1
	{ .magic = 0x200100420400a00ull, .mask = 0x8080808080876ull, .attacks = nullptr, .shift = 53, .table_size = 2048 }, // D1
	{ .magic = 0x100020410080100ull, .mask = 0x1010101010106eull, .attacks = nullptr, .shift = 53, .table_size = 2048 }, // E1
	{ .magic = 0x1080020080010400ull, .mask = 0x2020202020205eull, .attacks = nullptr, .shift = 53, .table_size = 2048 }, // F1
	{ .magic = 0x8400009001241228ull, .mask = 0x4040404040403eull, .attacks = nullptr, .shift = 53, .table_size = 2048 }, // G1
	{ .magic = 0x200002084004902ull, .mask = 0x8080808080807eull, .attacks = nullptr, .shift = 52, .table_size = 4096 }, // H1
	{ .magic = 0x800800080204001ull, .mask = 0x1010101017e00ull, .attacks = nullptr, .shift = 53, .table_size = 2048 }, // A2
	{ .magic = 0x400050002000ull, .mask = 0x2020202027c00ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // B2
	{ .magic = 0x1201802000811001ull, .mask = 0x4040404047a00ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // C2
	{ .magic = 0x211001000200900ull, .mask = 0x8080808087600ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // D2
	{ .magic = 0x800800800400ull, .mask = 0x10101010106e00ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // E2
	{ .magic = 0x408803200040080ull, .mask = 0x20202020205e00ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // F2
	{ .magic = 0x5504003214100108ull, .mask = 0x40404040403e00ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // G2
	{ .magic = 0x1001082005100ull, .mask = 0x80808080807e00ull, .attacks = nullptr, .shift = 53, .table_size = 2048 }, // H2
	{ .magic = 0x2920208000804004ull, .mask = 0x10101017e0100ull, .attacks = nullptr, .shift = 53, .table_size = 2048 }, // A3
	{ .magic = 0x190024000442000ull, .mask = 0x20202027c0200ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // B3
	{ .magic = 0x1900480200180b2ull, .mask = 0x40404047a0400ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // C3
	{ .magic = 0x109010008100420ull, .mask = 0x8080808760800ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // D3
	{ .magic = 0x4c008080040800ull, .mask = 0x101010106e1000ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // E3
	{ .magic = 0x80104401020ull, .mask = 0x202020205e2000ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // F3
	{ .magic = 0x40090420108ull, .mask = 0x404040403e4000ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // G3
	{ .magic = 0x304a0004124081ull, .mask = 0x808080807e8000ull, .attacks = nullptr, .shift = 53, .table_size = 2048 }, // H3
	{ .magic = 0x891a830200244201ull, .mask = 0x101017e010100ull, .attacks = nullptr, .shift = 53, .table_size = 2048 }, // A4
	{ .magic = 0x1004200220088ull, .mask = 0x202027c020200ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // B4
	{ .magic = 0x2000200480100180ull, .mask = 0x404047a040400ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // C4
	{ .magic = 0x9010001080800800ull, .mask = 0x8080876080800ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // D4
	{ .magic = 0xa040080800800ull, .mask = 0x1010106e101000ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // E4
	{ .magic = 0x4002004200702844ull, .mask = 0x2020205e202000ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // F4
	{ .magic = 0x1001000100040200ull, .mask = 0x4040403e404000ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // G4
	{ .magic = 0x8000042000c0091ull, .mask = 0x8080807e808000ull, .attacks = nullptr, .shift = 53, .table_size = 2048 }, // H4
	{ .magic = 0xc080002000400056ull, .mask = 0x1017e01010100ull, .attacks = nullptr, .shift = 53, .table_size = 2048 }, // A5
	{ .magic = 0x4000200080804002ull, .mask = 0x2027c02020200ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // B5
	{ .magic = 0x300100082802000ull, .mask = 0x4047a04040400ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // C5
	{ .magic = 0x10040800801080ull, .mask = 0x8087608080800ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // D5
	{ .magic = 0x8010041101000800ull, .mask = 0x10106e10101000ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // E5
	{ .magic = 0x31000401000208ull, .mask = 0x20205e20202000ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // F5
	{ .magic = 0x20800200800100ull, .mask = 0x40403e40404000ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // G5
	{ .magic = 0x8800940082000469ull, .mask = 0x80807e80808000ull, .attacks = nullptr, .shift = 53, .table_size = 2048 }, // H5
	{ .magic = 0x140802040008002ull, .mask = 0x17e0101010100ull, .attacks = nullptr, .shift = 53, .table_size = 2048 }, // A6
	{ .magic = 0x4000500020004000ull, .mask = 0x27c0202020200ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // B6
	{ .magic = 0x40100020008080ull, .mask = 0x47a0404040400ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // C6
	{ .magic = 0x601001000090020ull, .mask = 0x8760808080800ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // D6
	{ .magic = 0x84004080080800ull, .mask = 0x106e1010101000ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // E6
	{ .magic = 0x204004080110ull, .mask = 0x205e2020202000ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // F6
	{ .magic = 0x1008010002008080ull, .mask = 0x403e4040404000ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // G6
	{ .magic = 0xa00010040a20004ull, .mask = 0x807e8080808000ull, .attacks = nullptr, .shift = 53, .table_size = 2048 }, // H6
	{ .magic = 0x48fffe99fecfaa00ull, .mask = 0x7e010101010100ull, .attacks = nullptr, .shift = 54, .table_size = 2048 }, // A7
	{ .magic = 0x48fffe99fecfaa00ull, .mask = 0x7c020202020200ull, .attacks = nullptr, .shift = 55, .table_size = 512 }, // B7
	{ .magic = 0x497fffadff9c2e00ull, .mask = 0x7a040404040400ull, .attacks = nullptr, .shift = 55, .table_size = 512 }, // C7
	{ .magic = 0x613fffddffce9200ull, .mask = 0x76080808080800ull, .attacks = nullptr, .shift = 55, .table_size = 512 }, // D7
	{ .magic = 0xffffffe9ffe7ce00ull, .mask = 0x6e101010101000ull, .attacks = nullptr, .shift = 55, .table_size = 512 }, // E7
	{ .magic = 0xfffffff5fff3e600ull, .mask = 0x5e202020202000ull, .attacks = nullptr, .shift = 55, .table_size = 512 }, // F7
	{ .magic = 0x3ff95e5e6a4c0ull, .mask = 0x3e404040404000ull, .attacks = nullptr, .shift = 55, .table_size = 512 }, // G7
	{ .magic = 0x1006081040200ull, .mask = 0x7e808080808000ull, .attacks = nullptr, .shift = 53, .table_size = 2048 }, // H7
	{ .magic = 0xebffffb9ff9fc526ull, .mask = 0x7e01010101010100ull, .attacks = nullptr, .shift = 53, .table_size = 2048 }, // A8
	{ .magic = 0x61fffeddfeedaeaeull, .mask = 0x7c02020202020200ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // B8
	{ .magic = 0x53bfffedffdeb1a2ull, .mask = 0x7a04040404040400ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // C8
	{ .magic = 0x127fffb9ffdfb5f6ull, .mask = 0x7608080808080800ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // D8
	{ .magic = 0x411fffddffdbf4d6ull, .mask = 0x6e10101010101000ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // E8
	{ .magic = 0x11000204000801ull, .mask = 0x5e20202020202000ull, .attacks = nullptr, .shift = 53, .table_size = 2048 }, // F8
	{ .magic = 0x3ffef27eebe74ull, .mask = 0x3e40404040404000ull, .attacks = nullptr, .shift = 54, .table_size = 1024 }, // G8
	{ .magic = 0x7645fffecbfea79eull, .mask = 0x7e80808080808000ull, .attacks = nullptr, .shift = 53, .table_size = 2048 }, // H8
};

Magic BISHOP_MAGICS[64] = {
	{ .magic = 0xffedf9fd7cfcffffull, .mask = 0x40201008040200ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // A1
	{ .magic = 0xfc0962854a77f576ull, .mask = 0x402010080400ull, .attacks = nullptr, .shift = 60, .table_size = 16 }, // B1
	{ .magic = 0x6082209801000ull, .mask = 0x4020100a00ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // C1
	{ .magic = 0x3008a08200282080ull, .mask = 0x40221400ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // D1
	{ .magic = 0x10240420040a0000ull, .mask = 0x2442800ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // E1
	{ .magic = 0x114412010000000ull, .mask = 0x204085000ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // F1
	{ .magic = 0xfc0a66c64a7ef576ull, .mask = 0x20408102000ull, .attacks = nullptr, .shift = 60, .table_size = 16 }, // G1
	{ .magic = 0x7ffdfdfcbd79ffffull, .mask = 0x2040810204000ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // H1
	{ .magic = 0xfc0846a64a34fff6ull, .mask = 0x20100804020000ull, .attacks = nullptr, .shift = 60, .table_size = 16 }, // A2
	{ .magic = 0xfc087a874a3cf7f6ull, .mask = 0x40201008040000ull, .attacks = nullptr, .shift = 60, .table_size = 16 }, // B2
	{ .magic = 0x4002084805002202ull, .mask = 0x4020100a0000ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // C2
	{ .magic = 0x102824081000030ull, .mask = 0x4022140000ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // D2
	{ .magic = 0x200111041800000ull, .mask = 0x244280000ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // E2
	{ .magic = 0x4002808220a0e004ull, .mask = 0x20408500000ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // F2
	{ .magic = 0xfc0864ae59b4ff76ull, .mask = 0x2040810200000ull, .attacks = nullptr, .shift = 60, .table_size = 16 }, // G2
	{ .magic = 0x3c0860af4b35ff76ull, .mask = 0x4081020400000ull, .attacks = nullptr, .shift = 60, .table_size = 16 }, // H2
	{ .magic = 0x73c01af56cf4cffbull, .mask = 0x10080402000200ull, .attacks = nullptr, .shift = 60, .table_size = 16 }, // A3
	{ .magic = 0x41a01cfad64aaffcull, .mask = 0x20100804000400ull, .attacks = nullptr, .shift = 60, .table_size = 16 }, // B3
	{ .magic = 0x201058818010210ull, .mask = 0x4020100a000a00ull, .attacks = nullptr, .shift = 57, .table_size = 128 }, // C3
	{ .magic = 0x4802040420220081ull, .mask = 0x402214001400ull, .attacks = nullptr, .shift = 57, .table_size = 128 }, // D3
	{ .magic = 0x4020200a22604ull, .mask = 0x24428002800ull, .attacks = nullptr, .shift = 57, .table_size = 128 }, // E3
	{ .magic = 0x41000a00820108ull, .mask = 0x2040850005000ull, .attacks = nullptr, .shift = 57, .table_size = 128 }, // F3
	{ .magic = 0x7c0c028f5b34ff76ull, .mask = 0x4081020002000ull, .attacks = nullptr, .shift = 60, .table_size = 16 }, // G3
	{ .magic = 0xfc0a028e5ab4df76ull, .mask = 0x8102040004000ull, .attacks = nullptr, .shift = 60, .table_size = 16 }, // H3
	{ .magic = 0x40208400a0084200ull, .mask = 0x8040200020400ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // A4
	{ .magic = 0x8024008628804ull, .mask = 0x10080400040800ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // B4
	{ .magic = 0x40046610300400c0ull, .mask = 0x20100a000a1000ull, .attacks = nullptr, .shift = 57, .table_size = 128 }, // C4
	{ .magic = 0x8104040080401080ull, .mask = 0x40221400142200ull, .attacks = nullptr, .shift = 55, .table_size = 512 }, // D4
	{ .magic = 0x2488004002000ull, .mask = 0x2442800284400ull, .attacks = nullptr, .shift = 55, .table_size = 512 }, // E4
	{ .magic = 0x8888018080400ull, .mask = 0x4085000500800ull, .attacks = nullptr, .shift = 57, .table_size = 128 }, // F4
	{ .magic = 0x4004140809008202ull, .mask = 0x8102000201000ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // G4
	{ .magic = 0x242213004807800ull, .mask = 0x10204000402000ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // H4
	{ .magic = 0x101080840411120ull, .mask = 0x4020002040800ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // A5
	{ .magic = 0x4012008052409ull, .mask = 0x8040004081000ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // B5
	{ .magic = 0x2211149000020401ull, .mask = 0x100a000a102000ull, .attacks = nullptr, .shift = 57, .table_size = 128 }, // C5
	{ .magic = 0x4011400821020200ull, .mask = 0x22140014224000ull, .attacks = nullptr, .shift = 55, .table_size = 512 }, // D5
	{ .magic = 0x40110040040440ull, .mask = 0x44280028440200ull, .attacks = nullptr, .shift = 55, .table_size = 512 }, // E5
	{ .magic = 0x14004280141008ull, .mask = 0x8500050080400ull, .attacks = nullptr, .shift = 57, .table_size = 128 }, // F5
	{ .magic = 0x68020064040908ull, .mask = 0x10200020100800ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // G5
	{ .magic = 0x8001020080802400ull, .mask = 0x20400040201000ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // H5
	{ .magic = 0xdcefd9b54bfcc09full, .mask = 0x2000204081000ull, .attacks = nullptr, .shift = 60, .table_size = 16 }, // A6
	{ .magic = 0xf95ffa765afd602bull, .mask = 0x4000408102000ull, .attacks = nullptr, .shift = 60, .table_size = 16 }, // B6
	{ .magic = 0x402100443000c200ull, .mask = 0xa000a10204000ull, .attacks = nullptr, .shift = 57, .table_size = 128 }, // C6
	{ .magic = 0x3001002018080101ull, .mask = 0x14001422400000ull, .attacks = nullptr, .shift = 57, .table_size = 128 }, // D6
	{ .magic = 0x302202220c001200ull, .mask = 0x28002844020000ull, .attacks = nullptr, .shift = 57, .table_size = 128 }, // E6
	{ .magic = 0x2210010d0a0601ull, .mask = 0x50005008040200ull, .attacks = nullptr, .shift = 57, .table_size = 128 }, // F6
	{ .magic = 0x43ff9a5cf4ca0c01ull, .mask = 0x20002010080400ull, .attacks = nullptr, .shift = 60, .table_size = 16 }, // G6
	{ .magic = 0x4bffcd8e7c587601ull, .mask = 0x40004020100800ull, .attacks = nullptr, .shift = 60, .table_size = 16 }, // H6
	{ .magic = 0xfc0ff2865334f576ull, .mask = 0x20408102000ull, .attacks = nullptr, .shift = 60, .table_size = 16 }, // A7
	{ .magic = 0xfc0bf6ce5924f576ull, .mask = 0x40810204000ull, .attacks = nullptr, .shift = 60, .table_size = 16 }, // B7
	{ .magic = 0x8402204108214000ull, .mask = 0xa1020400000ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // C7
	{ .magic = 0x2100804042022110ull, .mask = 0x142240000000ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // D7
	{ .magic = 0x8102001020485010ull, .mask = 0x284402000000ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // E7
	{ .magic = 0x8054810010810ull, .mask = 0x500804020000ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // F7
	{ .magic = 0xc3ffb7dc36ca8c89ull, .mask = 0x201008040200ull, .attacks = nullptr, .shift = 60, .table_size = 16 }, // G7
	{ .magic = 0xc3ff8a54f4ca2c89ull, .mask = 0x402010080400ull, .attacks = nullptr, .shift = 60, .table_size = 16 }, // H7
	{ .magic = 0xfffffcfcfd79edffull, .mask = 0x2040810204000ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // A8
	{ .magic = 0xfc0863fccb147576ull, .mask = 0x4081020400000ull, .attacks = nullptr, .shift = 60, .table_size = 16 }, // B8
	{ .magic = 0x1140049201462211ull, .mask = 0xa102040000000ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // C8
	{ .magic = 0x1d00e00003084801ull, .mask = 0x14224000000000ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // D8
	{ .magic = 0x600050050620210ull, .mask = 0x28440200000000ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // E8
	{ .magic = 0xc102002102220a01ull, .mask = 0x50080402000000ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // F8
	{ .magic = 0xfc087e8e4bb2f736ull, .mask = 0x20100804020000ull, .attacks = nullptr, .shift = 60, .table_size = 16 }, // G8
	{ .magic = 0x43ff9e4ef4ca2c89ull, .mask = 0x40201008040200ull, .attacks = nullptr, .shift = 59, .table_size = 32 }, // H8
};

