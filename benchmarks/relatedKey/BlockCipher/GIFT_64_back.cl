# the implementation without "touint" function
@cipher GIFT_64

sbox uint4[16] s = {1, 10, 4, 12, 6, 15, 3, 9, 2, 13, 11, 7, 5, 0, 8, 14};
pbox uint[64] p = {0, 17, 34, 51, 48, 1, 18, 35, 32, 49, 2, 19, 16, 33, 50, 3, 4, 21, 38, 55, 52, 5, 22, 39, 36, 53, 6, 23, 20, 37, 54, 7, 8, 25, 42, 59, 56, 9, 26, 43, 40, 57, 10, 27, 24, 41, 58, 11, 12, 29, 46, 63, 60, 13, 30, 47, 44, 61, 14, 31, 28, 45, 62, 15};
#pbox uint[64] p = {63, 46, 29, 12, 15, 62, 45, 28, 31, 14, 61, 44, 47, 30, 13, 60, 59, 42, 25, 8, 11, 58, 41, 24, 27, 10, 57, 40, 43, 26, 9, 56, 55, 38, 21, 4, 7, 54, 37, 20, 23, 6, 53, 36, 39, 22, 5, 52, 51, 34, 17, 0, 3, 50, 33, 16, 19, 2, 49, 32, 35, 18, 1, 48};

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
    k6 = k6 >>> 2;
    k7 = k7 >>> 12;
    uint1[128] rtn;
    for (i from 0 to 15) {
        rtn[i] = k6[i];
        rtn[i + 16] = k7[i];
        rtn[i + 32] = k0[i];
        rtn[i + 48] = k1[i];
        rtn[i + 64] = k2[i];
        rtn[i + 80] = k3[i];
        rtn[i + 96] = k4[i];
        rtn[i + 112] = k5[i];
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

    p_out[63] = p_out[4 * i] ^ key[i + 16];
    p_out[62] = p_out[4 * i] ^ key[i + 16];
    p_out[59] = p_out[4 * i] ^ key[i + 16];
    p_out[58] = p_out[4 * i] ^ key[i + 16];
    p_out[55] = p_out[4 * i] ^ key[i + 16];
    p_out[54] = p_out[4 * i] ^ key[i + 16];
    p_out[51] = p_out[4 * i] ^ key[i + 16];
    p_out[50] = p_out[4 * i] ^ key[i + 16];
    p_out[47] = p_out[4 * i] ^ key[i + 16];
    p_out[46] = p_out[4 * i] ^ key[i + 16];
    p_out[43] = p_out[4 * i] ^ key[i + 16];
    p_out[42] = p_out[4 * i] ^ key[i + 16];
    p_out[39] = p_out[4 * i] ^ key[i + 16];
    p_out[38] = p_out[4 * i] ^ key[i + 16];
    p_out[35] = p_out[4 * i] ^ key[i + 16];
    p_out[34] = p_out[4 * i] ^ key[i + 16];
    p_out[31] = p_out[4 * i] ^ key[i + 16];
    p_out[30] = p_out[4 * i] ^ key[i + 16];
    p_out[27] = p_out[4 * i] ^ key[i + 16];
    p_out[26] = p_out[4 * i] ^ key[i + 16];
    p_out[23] = p_out[4 * i] ^ key[i + 16];
    p_out[22] = p_out[4 * i] ^ key[i + 16];
    p_out[19] = p_out[4 * i] ^ key[i + 16];
    p_out[18] = p_out[4 * i] ^ key[i + 16];
    p_out[15] = p_out[4 * i] ^ key[i + 16];
    p_out[14] = p_out[4 * i] ^ key[i + 16];
    p_out[11] = p_out[4 * i] ^ key[i + 16];
    p_out[10] = p_out[4 * i] ^ key[i + 16];
    p_out[7] = p_out[4 * i] ^ key[i + 16];
    p_out[6] = p_out[4 * i] ^ key[i + 16];
    p_out[3] = p_out[4 * i] ^ key[i + 16];
    p_out[2] = p_out[4 * i] ^ key[i + 16];

	for (i from 0 to 15) {
	    p_out[4 * i] = p_out[4 * i] ^ key[i + 16];
	    p_out[4 * i + 1] = p_out[4 * i + 1] ^ key[i];
	}
    # p_out[63] = p_out[63] ^ 1;
	#p_out[23] = p_out[23] ^ constants[r][5];
    #p_out[19] = p_out[19] ^ constants[r][4];
    #p_out[15] = p_out[15] ^ constants[r][3];
    #p_out[11] = p_out[11] ^ constants[r][2];
    #p_out[7] = p_out[7] ^ constants[r][1];
    #p_out[3] = p_out[3] ^ constants[r][0];
	return p_out;
}

fn uint1[64] enc(uint1[128] key, uint1[64] r_plaintext){
    for (i from 1 to 28) {
        r_plaintext = round_function(i, View(key, 0, 31), r_plaintext);
        key = key_schedule(key);
    }
    return r_plaintext;
}