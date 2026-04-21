@cipher Simeck_64_128

# Simeck is a SIMON-like Feistel cipher. The round function mirrors SIMON.cl
# except the three cyclic-shift constants are (R1=0, R2=5, R3=1) instead of
# SIMON's (1, 8, 2). See MILP_Division_Property-master/algorithm3/Simeck/simeck.py.
#
# pbox semantics in EasyBC: `rtn = p<X>` means bit X[i] lands at rtn[p[i]].
# So "left rotation by R" is encoded as p[i] = (i + R) mod word_len.

# p1 = left rotation by R1 = 0 (identity)
pbox uint[32] p1 = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
# p2 = left rotation by R2 = 5
pbox uint[32] p2 = {5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,0,1,2,3,4};
# p3 = left rotation by R3 = 1
pbox uint[32] p3 = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,0};

r_fn uint1[64] round_function(uint8 r, uint1[32] key, uint1[64] input) {
    uint1[32] l_input;
    uint1[32] r_input;
    for (i from 0 to 31) {
        l_input[i] = input[i];
        r_input[i] = input[i + 32];
    }
    uint1[32] p1_out = p1<l_input>;
    uint1[32] p2_out = p2<l_input>;
    uint1[32] p3_out = p3<l_input>;
    uint1[32] and_out;
    for (i from 0 to 31) {
        and_out[i] = p1_out[i] & p2_out[i];
    }
    uint1[32] l_out;
    for (i from 0 to 31) {
        l_out[i] = and_out[i] ^ p3_out[i] ^ r_input[i] ^ key[i];
    }
    uint1[64] rtn;
    for (i from 0 to 31) {
        rtn[i] = l_out[i];
        rtn[i + 32] = l_input[i];
    }
    return rtn;
}

fn uint1[64] enc(uint1[1344] key, uint1[64] r_plaintext){
    for (i from 1 to 42) {
        r_plaintext = round_function(i, View(key, (i - 1) * 32, i * 32 - 1), r_plaintext);
    }
    return r_plaintext;
}
