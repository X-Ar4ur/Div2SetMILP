# the implementation without "touint" function,
# and the XOR operation subjects are uint1 arrays other than bits
@cipher present_64

sbox uint4[16] s = {12, 5, 6, 11, 9, 0, 10, 13, 3, 14, 15, 8, 4, 7, 1, 2};
pbox uint[64] p = {0, 16, 32, 48, 1, 17, 33, 49, 2, 18, 34, 50, 3, 19, 35, 51, 4, 20, 36, 52, 5, 21, 37, 53, 6, 22, 38, 54, 7, 23, 39, 55, 8, 24, 40, 56, 9, 25, 41, 57, 10, 26, 42, 58, 11, 27, 43, 59, 12, 28, 44, 60, 13, 29, 45, 61, 14, 30, 46, 62, 15, 31, 47, 63};

k_schd uint1[80] key_schedule(uint6 roundNum uint1[80] key) {
    uint1[80] key_lsh = key <<< 61;
    uint1[4] sbox_in = View(key_lsh, 0, 3);
    uint1[4] sbox_out = s<sbox_in>;
    key_lsh[0] = sbox_out[0];
    key_lsh[1] = sbox_out[1];
    key_lsh[2] = sbox_out[2];
    key_lsh[3] = sbox_out[3];

    uint6 tk = touint(0, key_lsh[60], key_lsh[61], key_lsh[62], key_lsh[63], key_lsh[64]);
    tk = tk ^ roundNum;
    for (i from 0 to 4) {
        key_lsh[i+60] = tk[i+1];
    }
    #key_lsh[60] = key_lsh[60] ^ roundNum[1];
    #key_lsh[61] = key_lsh[61] ^ roundNum[2];
    #key_lsh[62] = key_lsh[62] ^ roundNum[3];
    #key_lsh[63] = key_lsh[63] ^ roundNum[4];
    #key_lsh[64] = key_lsh[64] ^ roundNum[5];
    return key_lsh;
}

r_fn uint1[64] round_function1(uint8 r, uint1[64] key, uint1[64] input) {
	uint1[64] n_input = input ^ key;
    uint1[64] s_out;
    for (i from 0 to 15) {
        uint1[4] sbox_in = View(n_input, i*4, i*4+3);
        uint1[4] sbox_out = s<sbox_in>;
        s_out[i * 4 + 0] = sbox_out[0];
        s_out[i * 4 + 1] = sbox_out[1];
        s_out[i * 4 + 2] = sbox_out[2];
        s_out[i * 4 + 3] = sbox_out[3];
	}
    uint1[64] rtn = p<s_out>;
	return rtn;
}

r_fn uint1[64] round_function2(uint8 r, uint1[64] key, uint1[64] input) {
    input = input ^ key;
    return input;
}

fn uint1[64] enc(uint1[80] key, uint1[64] r_plaintext){
    for (i from 1 to 31) {
        r_plaintext = round_function1(i, View(key, 0, 63), r_plaintext);
        key = key_schedule(i, key);
    }
    r_plaintext = round_function2(32, View(key, 0, 63), r_plaintext);
    key = key_schedule(i, key);
    return r_plaintext;
}