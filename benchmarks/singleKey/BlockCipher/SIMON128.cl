@cipher SIMON_128_128

pbox uint[64] p1 = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,0};
pbox uint[64] p2 = {8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,0,1,2,3,4,5,6,7};
pbox uint[64] p3 = {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,0,1};

r_fn uint1[128] round_function(uint8 r, uint1[64] key, uint1[128] input) {
    uint1[64] l_input;
    uint1[64] r_input;
    for (i from 0 to 63) {
        l_input[i] = input[i];
        r_input[i] = input[i + 64];
    }
    uint1[64] p1_out = p1<l_input>;
    uint1[64] p2_out = p2<l_input>;
    uint1[64] p3_out = p3<l_input>;
    uint1[64] and_out;
    for (i from 0 to 63) {
        and_out[i] = p1_out[i] & p2_out[i];
    }
    uint1[64] l_out;
    for (i from 0 to 63) {
        l_out[i] = and_out[i] ^ p3_out[i] ^ r_input[i] ^ key[i];
    }
    uint1[128] rtn;
    for (i from 0 to 63) {
        rtn[i] = l_out[i];
        rtn[i + 64] = l_input[i];
    }
	return rtn;
}

fn uint1[128] enc(uint1[4352] key, uint1[128] r_plaintext) {
    for (i from 1 to 68) {
        r_plaintext = round_function(i, View(key, (i - 1) * 64, i * 64 - 1), r_plaintext);
    }
    return r_plaintext;
}
