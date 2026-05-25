//
// Division Property MILP Constraint Generation Functions
// Algorithm 3: 2-subset bit-based division property
//

#ifndef EASYBC_DIVMILPCONS_H
#define EASYBC_DIVMILPCONS_H

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>

namespace DivMILPcons {

    // Division property XOR constraint:
    //   x_out - x_in1 - x_in2 = 0
    void divXorC(std::string path, int inputIdx1, int inputIdx2, int outputIdx);

    // Division property COPY constraint:
    //   x_in - copy1 - copy2 - ... - copyN = 0
    void divCopyC(std::string path, int inputIdx, std::vector<int> copyIdx);

    // Division property AND constraint:
    //   t - u >= 0
    //   t - v >= 0
    //   t - u - v <= 0
    void divAndC(std::string path, int inputIdx1, int inputIdx2, int outputIdx);

    // Division property S-box constraint:
    //   For each inequality [a0, ..., a_{2n-1}, b]:
    //   a0*x_in[0] + ... + a_{n-1}*x_in[n-1] + a_n*x_out[0] + ... + a_{2n-1}*x_out[n-1] >= -b
    void divSboxC(std::string path, std::vector<int> inputIdx, std::vector<int> outputIdx,
                  std::vector<std::vector<int>> ineqs);

    // Division property N-input XOR constraint (extension of divXorC for the
    // disjointed-representation linear-layer model):
    //   x_out - x_in1 - x_in2 - ... - x_inN = 0
    // inputIdx must be non-empty; N==1 is allowed and degenerates to an
    // aliasing equality x_out - x_in1 = 0.
    void divXorMultiC(std::string path, const std::vector<int>& inputIdx, int outputIdx);

}

#endif //EASYBC_DIVMILPCONS_H
