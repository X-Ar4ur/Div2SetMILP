#ifndef EASYBC_BDPTACTIVEBITS_H
#define EASYBC_BDPTACTIVEBITS_H

#include <string>
#include <vector>

std::vector<int> resolveBdptActiveBitVars(const std::string& cipherName,
                                          int blockSize,
                                          const std::string& activebitsSpec);

#endif // EASYBC_BDPTACTIVEBITS_H
