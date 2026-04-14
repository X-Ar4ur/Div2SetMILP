# the implementation without "touint" function,
# and the XOR operation subjects are uint1 arrays other than bits
@cipher TWINE_64

sbox uint4[16] s = {12, 0, 15, 10, 2, 11, 9, 5, 8, 3, 13, 7, 1, 14, 6, 4};
pbox uint[64] p = {20, 21, 22, 23, 0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 28, 29, 30, 31, 48, 49, 50, 51, 12, 13, 14, 15, 32, 33, 34, 35, 52, 53, 54, 55, 24, 25, 26, 27, 36, 37, 38, 39, 8, 9, 10, 11, 60, 61, 62, 63, 40, 41, 42, 43, 44, 45, 46, 47, 56, 57, 58, 59};

k_schd uint1[32] key_schedule1(uint1[80] key) {
    uint1[4] wk0 = View(key, 0, 3);
    uint1[4] wk1 = View(key, 4, 7);
    uint1[4] wk2 = View(key, 8, 11);
    uint1[4] wk3 = View(key, 12, 15);
    uint1[4] wk4 = View(key, 16, 19);

    uint1[4] wk5 = View(key, 20, 23);
    uint1[4] wk6 = View(key, 24, 27);
    uint1[4] wk7 = View(key, 28, 31);
    uint1[4] wk8 = View(key, 32, 35);
    uint1[4] wk9 = View(key, 36, 39);

    uint1[4] wk10 = View(key, 40, 43);
    uint1[4] wk11 = View(key, 44, 47);
    uint1[4] wk12 = View(key, 48, 51);
    uint1[4] wk13 = View(key, 52, 55);
    uint1[4] wk14 = View(key, 56, 59);

    uint1[4] wk15 = View(key, 60, 63);
    uint1[4] wk16 = View(key, 64, 67);
    uint1[4] wk17 = View(key, 68, 71);
    uint1[4] wk18 = View(key, 72, 75);
    uint1[4] wk19 = View(key, 76, 79);

    uint1[32] rtn;
    for (i from 0 to 3) {
        rtn[i] = wk1[i];
        rtn[i + 4] = wk3[i];
        rtn[i + 8] = wk4[i];
        rtn[i + 12] = wk6[i];
        rtn[i + 16] = wk13[i];
        rtn[i + 20] = wk14[i];
        rtn[i + 24] = wk15[i];
        rtn[i + 28] = wk16[i];
    }
    return rtn;
}

k_schd uint1[112] key_schedule2(uint1[80] key) {
    uint1[4] wk0 = View(key, 0, 3);
    uint1[4] wk1 = View(key, 4, 7);
    uint1[4] wk2 = View(key, 8, 11);
    uint1[4] wk3 = View(key, 12, 15);
    uint1[4] wk4 = View(key, 16, 19);

    uint1[4] wk5 = View(key, 20, 23);
    uint1[4] wk6 = View(key, 24, 27);
    uint1[4] wk7 = View(key, 28, 31);
    uint1[4] wk8 = View(key, 32, 35);
    uint1[4] wk9 = View(key, 36, 39);

    uint1[4] wk10 = View(key, 40, 43);
    uint1[4] wk11 = View(key, 44, 47);
    uint1[4] wk12 = View(key, 48, 51);
    uint1[4] wk13 = View(key, 52, 55);
    uint1[4] wk14 = View(key, 56, 59);

    uint1[4] wk15 = View(key, 60, 63);
    uint1[4] wk16 = View(key, 64, 67);
    uint1[4] wk17 = View(key, 68, 71);
    uint1[4] wk18 = View(key, 72, 75);
    uint1[4] wk19 = View(key, 76, 79);

    uint1[4] sbox_out0 = s<wk0>;
    uint1[4] sbox_out16 = s<wk16>;

    wk1 = wk1 ^ sbox_out0;
    wk4 = wk4 ^ sbox_out16;

    uint1[4] tmp0 = wk0;
    uint1[4] tmp1 = wk1; # 249
    uint1[4] tmp2 = wk2;
    uint1[4] tmp3 = wk3;

    wk0 = wk4; # 252
    wk1 = wk5;
    wk2 = wk6;
    wk3 = wk7;

    wk4 = wk8; # 33
    wk5 = wk9;
    wk6 = wk10;
    wk7 = wk11;

    wk8 = wk12;
    wk9 = wk13;
    wk10 = wk14;
    wk11 = wk15;

    wk12 = wk16;
    wk13 = wk17;
    wk14 = wk18;
    wk15 = wk19;

    wk16 = tmp1; # 249
    wk17 = tmp2;
    wk18 = tmp3;
    wk19 = tmp0;

    uint1[112] rtn;
    for (i from 0 to 3) {
        rtn[i] = wk1[i]; # 21
        rtn[i + 4] = wk3[i]; # 29
        rtn[i + 8] = wk4[i]; # 33
        rtn[i + 12] = wk6[i];# 41
        rtn[i + 16] = wk13[i]; # 69
        rtn[i + 20] = wk14[i]; # 73
        rtn[i + 24] = wk15[i]; # 77
        rtn[i + 28] = wk16[i]; # 249

        rtn[i + 32] = wk0[i];
        rtn[i + 36] = wk1[i];
        rtn[i + 40] = wk2[i];
        rtn[i + 44] = wk3[i];
        rtn[i + 48] = wk4[i];
        rtn[i + 52] = wk5[i];
        rtn[i + 56] = wk6[i];
        rtn[i + 60] = wk7[i];
        rtn[i + 64] = wk8[i];
        rtn[i + 68] = wk9[i];
        rtn[i + 72] = wk10[i];
        rtn[i + 76] = wk11[i];
        rtn[i + 80] = wk12[i];
        rtn[i + 84] = wk13[i];
        rtn[i + 88] = wk14[i];
        rtn[i + 92] = wk15[i];
        rtn[i + 96] = wk16[i];
        rtn[i + 100] = wk17[i];
        rtn[i + 104] = wk18[i];
        rtn[i + 108] = wk19[i];
    }
    return rtn;
}

r_fn uint1[64] round_function1(uint8 r, uint1[32] key, uint1[64] input) {
    uint1[64] n_input;
    for (i from 0 to 7) {
        for (j from 0 to 3) {
            n_input[i * 8 + j] = input[i * 8 + j] ^ key[i * 4 + j];
            n_input[i * 8 + j + 4] = input[i * 8 + j + 4];
        }
    }
    uint1[32] s_out;
    for (i from 0 to 7) {
        uint1[4] sbox_in = View(n_input, i*8, i*8+3);
        uint1[4] sbox_out = s<sbox_in>;
        s_out[i * 4 + 0] = sbox_out[0];
        s_out[i * 4 + 1] = sbox_out[1];
        s_out[i * 4 + 2] = sbox_out[2];
        s_out[i * 4 + 3] = sbox_out[3];
    }

    for (i from 0 to 7) {
        for (j from 0 to 3) {
            n_input[i * 8 + j + 4] = s_out[i * 4 + j] ^ n_input[i * 8 + j + 4];
            n_input[i * 8 + j] = input[i * 8 + j];
        }
    }
    uint1[64] p_out = p<n_input>;
	return p_out;
}

r_fn uint1[64] round_function2(uint8 r, uint1[32] key, uint1[64] input) {
    uint1[64] n_input;
    for (i from 0 to 7) {
        for (j from 0 to 3) {
            n_input[i * 8 + j] = input[i * 8 + j] ^ key[i * 4 + j];
            n_input[i * 8 + j + 4] = input[i * 8 + j + 4];
        }
    }
    uint1[32] s_out;
    for (i from 0 to 7) {
        uint1[4] sbox_in = View(n_input, i*8, i*8+3);
        uint1[4] sbox_out = s<sbox_in>;
        s_out[i * 4 + 0] = sbox_out[0];
        s_out[i * 4 + 1] = sbox_out[1];
        s_out[i * 4 + 2] = sbox_out[2];
        s_out[i * 4 + 3] = sbox_out[3];
    }

    for (i from 0 to 7) {
        for (j from 0 to 3) {
            n_input[i * 8 + j + 4] = s_out[i * 4 + j] ^ n_input[i * 8 + j + 4];
            n_input[i * 8 + j] = input[i * 8 + j];
        }
    }
    return n_input;
}

fn uint1[64] enc(uint1[80] key, uint1[64] r_plaintext){
    uint1[32] sub_keys = key_schedule1(key);
    r_plaintext = round_function1(i, sub_keys, r_plaintext);
    uint1[112] round_keys;
    round_keys = key_schedule2(key);
    r_plaintext = round_function1(i, View(round_keys, 0, 31), r_plaintext);
    for (i from 3 to 35) {
        round_keys = key_schedule2(View(round_keys, 32, 111));
        r_plaintext = round_function1(i, View(round_keys, 0, 31), r_plaintext);
    }
    round_keys = key_schedule2(View(round_keys, 32, 111));
    r_plaintext = round_function2(36, View(round_keys, 0, 31), r_plaintext);
    return r_plaintext;
}