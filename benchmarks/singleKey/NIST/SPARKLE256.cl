# # the permutation of Romulus : ARX-based S-box Alzette
@cipher SPARKLE256

pbox uint[32] p1 = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,0};
pbox uint[32] p2 = {8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,0,1,2,3,4,5,6,7};
pbox uint[32] p3 = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
pbox uint[32] p4 = {17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

uint32[8] constants = {3084996962, 3211876480, 951376470, 844003128, 3138487787, 1333558103, 3485442504, 3266521405};

uint1[32] c = {0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0};

s_fn uint1[64] sbox_function(uint1[64] input) {
    uint1[32] l_input = View(input, 0, 31);
    uint1[32] r_input = View(input, 32, 63);

    uint1[32] p1_out = r_input >>> 31;
    l_input = l_input + p1_out;
    uint1[32] p2_out = l_input >>> 24;
    r_input = r_input ^ p2_out;
    l_input = l_input ^ c;

    uint1[32] p3_out = r_input >>> 17;
    l_input = l_input + p3_out;
    uint1[32] p4_out = l_input >>> 17;
    r_input = r_input ^ p4_out;
    l_input = l_input ^ c;

    uint1[32] p5_out = r_input;
    l_input = l_input + p5_out;
    uint1[32] p6_out = l_input >>> 31;
    r_input = r_input ^ p6_out;
    l_input = l_input ^ c;

    uint1[32] p11_out1 = r_input >>> 24;
    l_input = l_input + p11_out1;
    uint1[32] p22_out1 = l_input >>> 16;
    r_input = r_input ^ p22_out1;
    l_input = l_input ^ c;

    uint1[64] rtn;
    for (i from 0 to 31) {
        rtn[i] = l_input[i];
        rtn[i + 32] = r_input[i];
    }
    return rtn;
}

r_fn uint1[256] round_function1(uint32 r, uint1[32] key, uint1[256] plaintext) {

    uint1[64] xy0;
    uint1[64] xy1;
    uint1[64] xy2;
    uint1[64] xy3;
    for (i from 0 to 63) {
        xy0[i] = plaintext[i];
        xy1[i] = plaintext[i+64];
        xy2[i] = plaintext[i+128];
        xy3[i] = plaintext[i+192];
    }

    #for (i from 0 to 31) {
    #    xy0[i+32] = xy0[i+32] ^ constants[r][i];
    #    xy1[i+32] = xy1[i+32] ^ r[i];
    #}

    uint1[64] nxy0;
    nxy0 = sbox_function(xy0);
    uint1[64] nxy1;
    nxy1 = sbox_function(xy1);
    uint1[64] nxy2;
    nxy2 = sbox_function(xy2);
    uint1[64] nxy3;
    nxy3 = sbox_function(xy3);

    uint1[64] tmp;
    for (i from 0 to 63) {
        tmp[i] = nxy0[i] ^ nxy1[i];
    }
    uint1[64] tmp1;
    for (i from 0 to 63) {
        tmp1[i] = tmp[i] ^ nxy2[i];
    }
    uint1[64] tmp2;
    for (i from 0 to 63) {
        tmp2[i] = tmp1[i] ^ nxy3[i];
    }

    uint1[32] tmpx;
    for (i from 0 to 31) {
        tmpx[i] = tmp2[i];
    }
    uint1[32] ltmp = tmpx <<< 16;
    uint1[32] l_tmp = ltmp ^ tmpx;

    uint1[32] tmpy;
    for (i from 0 to 31) {
        tmpy[i] = tmp2[i + 32];
    }
    uint1[32] rtmp = tmpy <<< 16;
    uint1[32] r_tmp = rtmp ^ tmpy;

    uint1[64] tmpout;
    for (i from 0 to 31) {
        tmpout[i] = l_tmp[i];
        tmpout[i+32] = r_tmp[i];
    }

    uint1[64] mxy0 = nxy0 ^ tmpout;
    uint1[64] mxy1 = nxy1 ^ tmpout;
    uint1[64] mxy2 = nxy2 ^ tmpout;
    uint1[64] mxy3 = nxy3 ^ tmpout;

    uint1[256] rtn;
    for (i from 0 to 63) {
        rtn[i] = mxy0[i];
        rtn[i + 64] = mxy1[i];
        rtn[i + 128] = mxy2[i];
        rtn[i + 192] = mxy3[i];
    }
    return rtn;
}



fn uint1[256] enc(uint1[64] key, uint1[256] r_plaintext){
    for (i from 1 to 12) {
        r_plaintext = round_function1(i, key, r_plaintext);
    }
    return r_plaintext;
}