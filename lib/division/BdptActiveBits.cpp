#include "division/BdptActiveBits.h"

#include <cassert>
#include <cctype>
#include <iostream>

std::vector<int> resolveBdptActiveBitVars(const std::string& cipherName,
                                          int blockSize,
                                          const std::string& activebitsSpec) {
    std::vector<int> active;
    if (activebitsSpec.empty() || blockSize <= 0) return active;

    const std::string& s = activebitsSpec;

    if (s.size() > 4 && s.substr(0, 4) == "hex:") {
        std::string hex = s.substr(4);
        if (hex.size() > 2 && hex[0] == '0' &&
            (hex[1] == 'x' || hex[1] == 'X')) {
            hex = hex.substr(2);
        }
        std::string clean;
        for (char c : hex) {
            if (c != '_' && c != ' ') clean.push_back(c);
        }
        int hexLen = (blockSize + 3) / 4;
        if ((int)clean.size() != hexLen) {
            std::cout << "ERROR: hex mask '" << clean << "' has "
                      << clean.size() << " nibbles; expected " << hexLen
                      << " for block size " << blockSize << std::endl;
            assert(false);
        }
        for (int nib = 0; nib < hexLen; ++nib) {
            char c = clean[nib];
            int v = 0;
            if (c >= '0' && c <= '9') v = c - '0';
            else if (c >= 'a' && c <= 'f') v = 10 + (c - 'a');
            else if (c >= 'A' && c <= 'F') v = 10 + (c - 'A');
            else {
                std::cout << "ERROR: non-hex char '" << c << "' in mask"
                          << std::endl;
                assert(false);
            }
            for (int b = 0; b < 4; ++b) {
                int xi = blockSize - nib * 4 - b;
                if (xi < 1) break;
                if ((v >> (3 - b)) & 1) active.push_back(xi);
            }
        }
        return active;
    }

    bool allDigits = !s.empty();
    for (char c : s) {
        if (!std::isdigit((unsigned char)c)) {
            allDigits = false;
            break;
        }
    }

    if (allDigits) {
        int n = std::stoi(s);
        if (n < 0 || n > blockSize) {
            std::cout << "ERROR: activebits " << n << " out of range [0, "
                      << blockSize << "]" << std::endl;
            assert(false);
        }

        if (cipherName == "Rectangle") {
            if (blockSize != 64) {
                std::cout << "ERROR: Rectangle preset expects block size 64, got "
                          << blockSize << std::endl;
                assert(false);
            }
            // Rectangle 沿用 Div2SetMILP/rectangle.py 的 4x16 bit-slice 初始布局。
            for (int i = 0; i < n; ++i) {
                int row = (i + 2) % 4;
                int col = 15 - (i / 4);
                active.push_back(row * 16 + col + 1);
            }
            return active;
        }

        if (cipherName == "LBlock") {
            int wordLen = blockSize / 2;
            if (wordLen != 32) {
                std::cout << "ERROR: LBlock preset expects block size 64, got "
                          << blockSize << std::endl;
                assert(false);
            }
            auto halfIdx = [&](int i) -> int {
                return (7 - (i / 4)) * 4 + (i % 4);
            };
            int yActive = std::min(n, 32);
            int xActive = std::max(0, n - 32);
            for (int i = 0; i < yActive; ++i) {
                active.push_back(wordLen + halfIdx(i) + 1);
            }
            for (int i = 0; i < xActive; ++i) {
                active.push_back(halfIdx(i) + 1);
            }
            return active;
        }

        for (int i = 0; i < n; ++i) active.push_back(blockSize - i);
        return active;
    }

    int wordLen = blockSize / 2;
    int lCount = 0;
    int rCount = 0;
    size_t p = 0;

    if (p < s.size() && s[p] == 'L') {
        ++p;
        size_t start = p;
        while (p < s.size() && std::isdigit((unsigned char)s[p])) ++p;
        if (start == p) {
            std::cout << "ERROR: malformed L<m> in '" << s << "'"
                      << std::endl;
            assert(false);
        }
        lCount = std::stoi(s.substr(start, p - start));
    }
    if (p < s.size() && s[p] == 'R') {
        ++p;
        size_t start = p;
        while (p < s.size() && std::isdigit((unsigned char)s[p])) ++p;
        if (start == p) {
            std::cout << "ERROR: malformed R<k> in '" << s << "'"
                      << std::endl;
            assert(false);
        }
        rCount = std::stoi(s.substr(start, p - start));
    }
    if (p != s.size() || (lCount == 0 && rCount == 0)) {
        std::cout << "ERROR: unrecognized activebits spec '" << s << "'. "
                  << "Use <N>, R<k>, L<m>, L<m>R<k>, or hex:<mask>."
                  << std::endl;
        assert(false);
    }
    if (lCount < 0 || lCount > wordLen || rCount < 0 || rCount > wordLen) {
        std::cout << "ERROR: L/R counts out of range [0, " << wordLen
                  << "] in '" << s << "'" << std::endl;
        assert(false);
    }

    for (int i = 0; i < rCount; ++i) active.push_back(blockSize - i);
    for (int i = 0; i < lCount; ++i) active.push_back(wordLen - i);
    return active;
}
