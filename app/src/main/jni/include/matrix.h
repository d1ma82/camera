/*
 *  2022 NDK Camera2 API project
 *
 *  Author: Zverintsev Dima
 * 
 *  Basic Matrix manipulations
 */

#ifndef _MATRIX_H
#define _MATRIX_H

#include <sstream>
#include "log.h"

namespace mtx {

    template <typename T, uint32_t _cols, uint32_t _rows>
    struct mat {
        T data[_cols * _rows];
        uint32_t rows = _rows;
        uint32_t cols = _cols; 
        inline T& operator[](uint32_t i) {return data[i];}
    };

    /*
                4x4 matrix only
            a1 a2       b1 b2       r1 = a1*b1 + a2*b3  r2 = a1*b2 + a2*b4
            a3 a4   x   b3 b4       r3 = a3*b1 + a4*b3  r4 = a3*b2 + a4*b4
            ...................

    */
    template<typename T, uint32_t a_cols, uint32_t a_rows, uint32_t b_cols, uint32_t b_rows>
    mat<T, b_cols, a_rows> operator*(mat<T, a_cols, a_rows>& a, mat<T, b_cols, b_rows>& b) {

        ASSERT(a_cols == b_rows, "Invalid matrix")

        mtx::mat<T, b_cols, a_rows> result;
        int32_t bi = 0, ai = 0, k = 0;
        for (int32_t i=0; i<b_cols*a_rows; i++) {
        
            T i1 = a[ai++] * b[bi];
            T i2 = a[ai++] * b[bi+b_cols];
            T i3 = a[ai++] * b[bi+2*b_cols];
            T i4 = a[ai++] * b[bi+3*b_cols];
            if (++bi == 4) { k+=4; ai=k; bi=0; } else { ai=k; }
            result[i] = i1 + i2 + i3 + i4;
        };
        return result;
    }

    template<typename T, uint32_t cols, uint32_t rows>
    const char* to_string(mat<T, cols, rows>& mat) {

        std::stringstream out;
        out << "[[";

        for (int i=0; i<cols*rows; i++) {

            out << mat[i];
            if ((i+1) % 4 == 0) out << (i+1 < cols*rows ? "], [": "]"); else out << ", ";
        }
        out << "]";

        return out.str().c_str();
    }
}
#endif