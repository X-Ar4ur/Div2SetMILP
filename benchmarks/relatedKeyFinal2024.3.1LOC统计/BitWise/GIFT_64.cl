# the implementation without "touint" function
@cipher GIFT_64

sbox uint4[16] s = {1, 10, 4, 12, 6, 15, 3, 9, 2, 13, 11, 7, 5, 0, 8, 14};
pbox uint[64] p = {0, 17, 34, 51, 48, 1, 18, 35, 32, 49, 2, 19, 16, 33, 50, 3, 4, 21, 38, 55, 52, 5, 22, 39, 36, 53, 6, 23, 20, 37, 54, 7, 8, 25, 42, 59, 56, 9, 26, 43, 40, 57, 10, 27, 24, 41, 58, 11, 12, 29, 46, 63, 60, 13, 30, 47, 44, 61, 14, 31, 28, 45, 62, 15};
uint6[48] constants = {1, 3, 7, 15, 31, 62, 61, 60, 55, 47, 30, 60, 57, 51, 39, 14, 29, 58, 53, 43, 32, 44, 24, 48, 33, 2, 5, 11, 23, 46, 28, 56, 49, 35, 6, 13, 27, 54, 45, 26, 52, 41, 18, 36, 8, 17, 34, 4, 9, 19, 38, 12, 25, 50, 37, 10, 21, 42, 20, 40, 16, 32};

k_schd uint1[128] key_schedule(uint1[128] key) {
    uint1[16] k0 = View(key, 0, 15);
    uint1[16] k1 = View(key, 16, 31);
    uint1[16] k2 = View(key, 32, 47);
    uint1[16] k3 = View(key, 48, 63);
    uint1[16] k4 = View(key, 64, 79);
    uint1[16] k5 = View(key, 80, 95);
    uint1[16] k6 = View(key, 96, 111);
    uint1[16] k7 = View(key, 112, 127);
    k1 = k1 <<< 2;
    k0 = k0 <<< 12;
    uint1[128] rtn;
    for (i from 0 to 15) {
        rtn[i] = k2[i];
        rtn[i + 16] = k3[i];
        rtn[i + 32] = k4[i];
        rtn[i + 48] = k5[i];
        rtn[i + 64] = k6[i];
        rtn[i + 80] = k7[i];
        rtn[i + 96] = k0[i];
        rtn[i + 112] = k1[i];
    }
    return rtn;
}

r_fn uint1[64] round_function(uint8 r, uint1[32] key, uint1[64] input) {
    uint1[64] s_out;
	for (i from 0 to 15) {
	    uint1[4] sbox_in = View(input, i*4, i*4+3);
        uint1[4] sbox_out = s<sbox_in>;
        s_out[i * 4] = sbox_out[0];
        s_out[i * 4 + 1] = sbox_out[1];
        s_out[i * 4 + 2] = sbox_out[2];
        s_out[i * 4 + 3] = sbox_out[3];
	}
    uint1[64] p_out = p<s_out>;
	for (i from 0 to 15) {
	    p_out[4 * i] = p_out[4 * i] ^ key[i];
	    p_out[4 * i + 1] = p_out[4 * i + 1] ^ key[i + 16];
	}
	return p_out;
}

fn uint1[64] enc(uint1[128] key, uint1[64] r_plaintext){
    for (i from 1 to 28) {
        r_plaintext = round_function(i, View(key, 0, 31), r_plaintext);
        key = key_schedule(key);
    }
    return r_plaintext;
}