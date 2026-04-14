@cipher KLEIN2

sbox uint4[16] s = {7, 4, 10, 9, 1, 15, 11, 0, 12, 3, 2, 6, 8, 14, 13, 5};
pbox uint[64] p = {16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
pboxm uint8[4][4] M = {{2, 3, 1, 1},
                        {1, 2, 3, 1},
                        {1, 1, 2, 3},
                        {3, 1, 1, 2}};
ffm uint4[16][16] M = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                      {0, 2, 4, 6, 8, 10, 12, 14, 3, 1, 7, 5, 11, 9, 15, 13},
                      {0, 3, 6, 5, 12, 15, 10, 9, 11, 8, 13, 14, 7, 4, 1, 2},
                      {0, 4, 8, 12, 3, 7, 11, 15, 6, 2, 14, 10, 5, 1, 13, 9},
                      {0, 5, 10, 15, 7, 2, 13, 8, 14, 11, 4, 1, 9, 12, 3, 6},
                      {0, 6, 12, 10, 11, 13, 7, 1, 5, 3, 9, 15, 14, 8, 2, 4},
                      {0, 6, 12, 10, 11, 13, 7, 1, 5, 3, 9, 15, 14, 8, 2, 4},
                      {0, 8, 3, 11, 6, 14, 5, 13, 12, 4, 15, 7, 10, 2, 9, 1},
                      {0, 9, 1, 8, 2, 11, 3, 10, 4, 13, 5, 12, 6, 15, 7, 14},
                      {0, 10, 7, 13, 14, 4, 9, 3, 15, 5, 8, 2, 1, 11, 6, 12},
                      {0, 11, 5, 14, 10, 1, 15, 4, 7, 12, 2, 9, 13, 6, 8, 3},
                      {0, 12, 11, 7, 5, 9, 14, 2, 10, 6, 1, 13, 15, 3, 4, 8},
                      {0, 13, 9, 4, 1, 12, 8, 5, 2, 15, 11, 6, 3, 14, 10, 7},
                      {0, 14, 15, 1, 13, 3, 2, 12, 9, 7, 6, 8, 4, 10, 11, 5},
                      {0, 15, 13, 2, 9, 6, 4, 11, 1, 14, 12, 3, 8, 7, 5, 10}};

k_schd uint1[64] key_schedule(uint1[64] key) {
    uint1[32] a = View(key, 0, 31);
    uint1[32] b = View(key, 32, 63);
    a = a <<< 8;
    b = b <<< 8;
    uint1[32] a1 = b;
    uint1[32] b1 = a ^ b;
    uint1[64] rtn;
    for (i from 0 to 31) {
        rtn[i] = a1[i];
        rtn[i + 32] = b1[i];
    }
    uint1[4] sbox_in1 = View(rtn, 40, 43);
    uint1[4] sbox_in2 = View(rtn, 44, 47);
    uint1[4] sbox_in3 = View(rtn, 48, 51);
    uint1[4] sbox_in4 = View(rtn, 52, 55);
    uint1[4] sbox_out1 = s<sbox_in1>;
    uint1[4] sbox_out2 = s<sbox_in2>;
    uint1[4] sbox_out3 = s<sbox_in3>;
    uint1[4] sbox_out4 = s<sbox_in4>;
    for (i from 0 to 3) {
        rtn[i + 40] = sbox_out1[i];
        rtn[i + 44] = sbox_out2[i];
        rtn[i + 48] = sbox_out3[i];
        rtn[i + 52] = sbox_out4[i];
    }
    return rtn;
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
    uint1[64] p_out = p<s_out>;
    uint8[4] m_in1;
    uint8[4] m_in2;
    for (i from 0 to 3) {
        m_in1[i] = touint(p_out[i*4], p_out[i*4+1], p_out[i*4+2], p_out[i*4+3],
                   p_out[i*4+4], p_out[i*4+5], p_out[i*4+6], p_out[i*4+7]);
    }
    for (i from 4 to 7) {
        m_in2[i] = touint(p_out[i*4], p_out[i*4+1], p_out[i*4+2], p_out[i*4+3],
                   p_out[i*4+4], p_out[i*4+5], p_out[i*4+6], p_out[i*4+7]);
    }
    uint8[4] m_out1 = M * m_in1;
    uint8[4] m_out2 = M * m_in2;
    uint1[64] rtn;
    for (i from 0 to 3) {
        for (j from 0 to 7) {
            rtn[i*8+j] = m_out1[i][j];
            rtn[i*8+j+32] = m_out2[i][j];
        }
    }

	return rtn;
}

r_fn uint1[64] round_function2(uint8 r, uint1[64] key, uint1[64] input) {
    input = input ^ key;
    return input;
}

fn uint1[64] enc(uint1[64] key, uint1[64] r_plaintext){
    for (i from 1 to 12) {
        r_plaintext = round_function1(i, key, r_plaintext);
        key = key_schedule(key);
    }
    r_plaintext = round_function2(13, key, r_plaintext);
    return r_plaintext;
}