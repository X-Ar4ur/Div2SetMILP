@cipher Simeck_32_64

# Simeck is a SIMON-like Feistel cipher. The round function uses cyclic
# shifts (R1=0, R2=5, R3=1).

# p1 = left rotation by R1 = 0 (identity)
pbox uint[16] p1 = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
# p2 = left rotation by R2 = 5
pbox uint[16] p2 = {5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4};
# p3 = left rotation by R3 = 1
pbox uint[16] p3 = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0};

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

fn uint1[32] enc(uint1[672] key, uint1[32] r_plaintext){
    for (i from 1 to 32) {
        r_plaintext = round_function(i, View(key, (i - 1) * 16, i * 16 - 1), r_plaintext);
    }
    return r_plaintext;
}
