@cipher nodeCounterTest

sbox uint4[16] s1 = {13, 10, 15, 0, 14, 4, 9, 11, 2, 1, 8, 3, 7, 5, 12, 6};

r_fn uint4 round_function(uint8 r, uint4 key, uint4 input) {
    uint4 s_out = s1<input>;
    uint4 rtn = s_out ^ key;
	return rtn;
}
