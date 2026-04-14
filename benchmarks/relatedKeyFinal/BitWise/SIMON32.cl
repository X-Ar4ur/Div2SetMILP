@cipher SIMON_32_64

pbox uint[16] p1 = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0};
pbox uint[16] p2 = {8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7};
pbox uint[16] p3 = {2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1};

k_schd uint1[64] key_schedule(uint1[64] key) {
    uint1[16] k1 = View(key, 0, 15);
    uint1[16] k2 = View(key, 16, 31);
    uint1[16] k3 = View(key, 32, 47);
    uint1[16] k4 = View(key, 48, 63);

    uint1[16] k12 = k1 ^ k2;
    uint1[16] k2_r1 = k2 >>> 1;
    uint1[16] k4_r3 = k4 >>> 3;
    uint1[16] k4_r4 = k4 >>> 4;
    uint1[16] k5 = k12 ^ k2_r1;
    k5 = k5 ^ k4_r3;
    k5 = k5 ^ k4_r4;

    uint1[64] rtn;
    for (i from 0 to 15) {
        rtn[i] = k2[i];
        rtn[i + 16] = k3[i];
        rtn[i + 32] = k4[i];
        rtn[i + 48] = k5[i];
    }
    return rtn;
}

r_fn uint1[32] round_function(uint8 r, uint1[16] key, uint1[32] input) {
    uint1[16] l_input;
    uint1[16] r_input;
    for (i from 0 to 15) {
        l_input[i] = input[i];
        r_input[i] = input[i + 16];
    }
    uint1[16] p1_out = p1<l_input>;
    uint1[16] p2_out = p2<l_input>;
    uint1[16] p3_out = p3<l_input>;
    uint1[16] and_out;
    for (i from 0 to 15) {
        and_out[i] = p1_out[i] & p2_out[i];
    }
    uint1[16] l_out;
    for (i from 0 to 15) {
        l_out[i] = and_out[i] ^ p3_out[i] ^ r_input[i] ^ key[i];
    }
    uint1[32] rtn;
    for (i from 0 to 15) {
        rtn[i] = l_out[i];
        rtn[i + 16] = l_input[i];
    }
	return rtn;
}

fn uint1[32] enc(uint1[64] key, uint1[32] r_plaintext){
    r_plaintext = round_function(1, View(key, 0, 15), r_plaintext);
    r_plaintext = round_function(2, View(key, 16, 31), r_plaintext);
    r_plaintext = round_function(3, View(key, 32, 47), r_plaintext);
    r_plaintext = round_function(4, View(key, 48, 63), r_plaintext);
    for (i from 5 to 32) {
        key = key_schedule(key);
        r_plaintext = round_function(i, View(key, 48, 63), r_plaintext);
    }
    return r_plaintext;
}