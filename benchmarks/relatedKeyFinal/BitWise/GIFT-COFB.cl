@cipher GIFT_128

sbox uint4[16] s = {1, 10, 4, 12, 6, 15, 3, 9, 2, 13, 11, 7, 5, 0, 8, 14};
pbox uint[128] p = {99, 66, 33, 0, 67, 34, 1, 96, 35, 2, 97, 64, 3, 98, 65, 32, 103, 70, 37, 4, 71, 38, 5, 100, 39, 6, 101, 68, 7, 102, 69, 36, 107, 74, 41, 8, 75, 42, 9, 104, 43, 10, 105, 72, 11, 106, 73, 40, 111, 78, 45, 12, 79, 46, 13, 108, 47, 14, 109, 76, 15, 110, 77, 44, 115, 82, 49, 16, 83, 50, 17, 112, 51, 18, 113, 80, 19, 114, 81, 48, 119, 86, 53, 20, 87, 54, 21, 116, 55, 22, 117, 84, 23, 118, 85, 52, 123, 90, 57, 24, 91, 58, 25, 120, 59, 26, 121, 88, 27, 122, 89, 56, 127, 94, 61, 28, 95, 62, 29, 124, 63, 30, 125, 92, 31, 126, 93, 60};
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

r_fn uint1[128] round_function(uint8 r, uint1[64] key, uint1[128] input) {
    uint1[128] s_out;
	for (i from 0 to 31) {
	    uint1[4] sbox_in = {input[i*4+3], input[i*4+2], input[i*4+1], input[i*4]};
        uint1[4] sbox_out = s<sbox_in>;
        s_out[i * 4] = sbox_out[0];
        s_out[i * 4 + 1] = sbox_out[1];
        s_out[i * 4 + 2] = sbox_out[2];
        s_out[i * 4 + 3] = sbox_out[3];
	}
    uint1[128] p_out = p<s_out>;
	for (i from 0 to 31) {
	    p_out[4 * i + 1] = p_out[4 * i + 1] ^ key[i];
	    p_out[4 * i] = p_out[4 * i] ^ key[i + 16];
	}
    #p_out[127] = p_out[127] ^ 1;
	#p_out[23] = p_out[23] ^ constants[r][5];
    #p_out[19] = p_out[19] ^ constants[r][4];
    #p_out[15] = p_out[15] ^ constants[r][3];
    #p_out[11] = p_out[11] ^ constants[r][2];
    #p_out[7] = p_out[7] ^ constants[r][1];
    #p_out[3] = p_out[3] ^ constants[r][0];
	return p_out;
}

fn uint1[128] enc(uint1[128] key, uint1[128] r_plaintext){
    uint1[64] round_key;
    for (i from 0 to 31) {
        round_key[i] = key[i];
    }
    for (i from 32 to 63) {
        round_key[i] = key[i + 32];
    }
    for (i from 1 to 40) {
        r_plaintext = round_function(i, round_key, r_plaintext);
        key = key_schedule(key);
        for (i from 0 to 31) {
            round_key[i] = key[i];
        }
        for (i from 32 to 63) {
            round_key[i] = key[i + 32];
        }
    }
    return r_plaintext;
}