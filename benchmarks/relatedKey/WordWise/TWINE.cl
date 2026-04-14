@cipher TWINE

sbox uint4[16] s = {12, 0, 15, 10, 2, 11, 9, 5, 8, 3, 13, 7, 1, 14, 6, 4};
pbox uint[16] p = {5,0,1,4,7,12,3,8,13,6,9,2,15,10,11,14};

k_schd uint4[288] key_schedule(uint4[20] key) {
    uint4[288] rtn;
    rtn[0] = key[1];
    rtn[1] = key[3];
    rtn[2] = key[4];
    rtn[3] = key[6];
    rtn[4] = key[13];
    rtn[5] = key[14];
    rtn[6] = key[15];
    rtn[7] = key[16];

    for (i from 1 to 35) {
        uint4 sbox_out0 = s<key[0]>;
        uint4 sbox_out16 = s<key[16]>;

        uint4 key1 = key[1] ^ sbox_out0;
        uint4 key4 = key[4] ^ sbox_out16;

        uint4 tmp0 = key[0];
        uint4 tmp1 = key1;
        uint4 tmp2 = key[2];
        uint4 tmp3 = key[3];

        key[0] = key4;
        key[1] = key[5];
        key[2] = key[6];
        key[3] = key[7];

        key[4] = key[8];
        key[5] = key[9];
        key[6] = key[10];
        key[7] = key[11];

        key[8] = key[12];
        key[9] = key[13];
        key[10] = key[14];
        key[11] = key[15];

        key[12] = key[16];
        key[13] = key[17];
        key[14] = key[18];
        key[15] = key[19];

        key[16] = tmp1;
        key[17] = tmp2;
        key[18] = tmp3;
        key[19] = tmp0;

        rtn[i * 8] = key[1];
        rtn[i * 8 + 1] = key[3];
        rtn[i * 8 + 2] = key[4];
        rtn[i * 8 + 3] = key[6];
        rtn[i * 8 + 4] = key[13];
        rtn[i * 8 + 5] = key[14];
        rtn[i * 8 + 6] = key[15];
        rtn[i * 8 + 7] = key[16];
    }
    return rtn;
}

r_fn uint4[16] round_function1(uint8 r, uint4[8] key, uint4[16] input) {
    uint4[16] n_input;
    for (i from 0 to 7) {
        n_input[i * 2] = input[i * 2] ^ key[i];
        n_input[i * 2 + 1] = input[i * 2 + 1];
    }

    uint4[8] s_out;
    for (i from 0 to 7) {
        s_out[i] = s<n_input[i * 2]>;
    }

    for (i from 0 to 7) {
        n_input[i * 2 + 1] = s_out[i] ^ n_input[i * 2 + 1];
        n_input[i * 2] = input[i * 2];
    }
    uint4[16] p_out = p<n_input>;
	return p_out;
}

r_fn uint4[16] round_function2(uint8 r, uint4[8] key, uint4[16] input) {
    uint4[16] n_input;
    for (i from 0 to 7) {
        n_input[i * 2] = input[i * 2] ^ key[i];
        n_input[i * 2 + 1] = input[i * 2 + 1];
    }

    uint4[8] s_out;
    for (i from 0 to 7) {
        s_out[i] = s<n_input[i * 2]>;
    }

    for (i from 0 to 7) {
        n_input[i * 2 + 1] = s_out[i] ^ n_input[i * 2 + 1];
        n_input[i * 2] = input[i * 2];
    }
    return n_input;
}

fn uint4[16] enc(uint4[20] key, uint4[16] r_plaintext){
    uint4[288] sub_keys = key_schedule(key);
    for (i from 1 to 35) {
        r_plaintext = round_function1(i, View(sub_keys, (i - 1) * 8, i * 8 - 1), r_plaintext);
    }
    r_plaintext = round_function2(36, View(sub_keys, 280, 287), r_plaintext);
    return r_plaintext;
}