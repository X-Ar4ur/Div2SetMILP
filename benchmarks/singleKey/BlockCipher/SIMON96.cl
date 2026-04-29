@cipher SIMON_96_96

pbox uint[48] p1 = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,0};
pbox uint[48] p2 = {8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,0,1,2,3,4,5,6,7};
pbox uint[48] p3 = {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,0,1};

r_fn uint1[96] round_function(uint8 r, uint1[48] key, uint1[96] input) {
    uint1[48] l_input;
    uint1[48] r_input;
    for (i from 0 to 47) {
        l_input[i] = input[i];
        r_input[i] = input[i + 48];
    }
    uint1[48] p1_out = p1<l_input>;
    uint1[48] p2_out = p2<l_input>;
    uint1[48] p3_out = p3<l_input>;
    uint1[48] and_out;
    for (i from 0 to 47) {
        and_out[i] = p1_out[i] & p2_out[i];
    }
    uint1[48] l_out;
    for (i from 0 to 47) {
        l_out[i] = and_out[i] ^ p3_out[i] ^ r_input[i] ^ key[i];
    }
    uint1[96] rtn;
    for (i from 0 to 47) {
        rtn[i] = l_out[i];
        rtn[i + 48] = l_input[i];
    }
	return rtn;
}

fn uint1[96] enc(uint1[2496] key, uint1[96] r_plaintext) {
    for (i from 1 to 52) {
        r_plaintext = round_function(i, View(key, (i - 1) * 48, i * 48 - 1), r_plaintext);
    }
    return r_plaintext;
}
