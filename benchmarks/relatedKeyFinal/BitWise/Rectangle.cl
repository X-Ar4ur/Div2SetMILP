# the implementation without "touint" function,
# and the XOR operation subjects are uint1 arrays other than bits
@cipher Rectangle_64

sbox uint4[16] s = {6, 5, 12, 10, 1, 14, 7, 9, 11, 0, 3, 13, 8, 15, 4, 2};
pbox uint[64] p = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                   31, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
                   36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 32, 33, 34, 35,
                   51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 48, 49, 50};

k_schd uint1[80] key_schedule(uint1[80] key) {
    uint1[16] k0 = View(key, 0, 15);
    uint1[16] k1 = View(key, 16, 31);
    uint1[16] k2 = View(key, 32, 47);
    uint1[16] k3 = View(key, 48, 63);
    uint1[16] k4 = View(key, 64, 79);
    for (i from 0 to 3) {
        uint4 sbox_in = touint(k3[i], k2[i], k1[i], k0[i]);
        uint4 sbox_out = s<sbox_in>;
        k3[i] = sbox_out[3];
        k2[i] = sbox_out[2];
        k1[i] = sbox_out[1];
        k0[i] = sbox_out[0];
    }
    uint1[16] nk0 = k0 >>> 8;
    uint1[16] nk1 = k2;
    uint1[16] nk2 = k3;
    uint1[16] nk3 = k3 >>> 12;
    uint1[16] nk4 = k0;
    nk0 = nk0 ^ k1;
    nk3 = nk3 ^ k4;

    uint1[80] rtn;
    for (i from 0 to 15) {
        rtn[i] = nk0[i];
        rtn[i + 16] = nk1[i];
        rtn[i + 32] = nk2[i];
        rtn[i + 48] = nk3[i];
        rtn[i + 64] = nk4[i];
    }
    return rtn;
}

r_fn uint1[64] round_function1(uint8 r, uint1[64] key, uint1[64] input) {
	uint1[64] n_input = input ^ key;
    uint1[64] s_out;
	for (i from 0 to 15) {
        uint1[4] sbox_in = {n_input[i], n_input[i+16], n_input[i+32], n_input[i+48]};
        uint1[4] sbox_out = s<sbox_in>;
        s_out[i] = sbox_out[0];
        s_out[i + 16] = sbox_out[1];
        s_out[i + 32] = sbox_out[2];
        s_out[i + 48] = sbox_out[3];
    }
    uint1[64] rtn = p<s_out>;
	return rtn;
}

r_fn uint1[64] round_function2(uint8 r, uint1[64] key, uint1[64] input) {
    uint1[64] rtn = input ^ key;
    return rtn;
}

fn uint1[64] enc(uint1[80] key, uint1[64] r_plaintext){
    for (i from 1 to 25) {
        r_plaintext = round_function1(i, View(key, 0, 63), r_plaintext);
        key = key_schedule(key);
    }
    r_plaintext = round_function2(26, View(key, 0, 63), r_plaintext);
    return r_plaintext;
}