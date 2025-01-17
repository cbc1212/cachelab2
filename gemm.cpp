/*
请注意，你的代码不能出现任何 int/short/char/float/double/auto 等局部变量/函数传参，我们仅允许使用 reg 定义的寄存器变量。
其中 reg 等价于一个 int。

你不能自己申请额外的内存，即不能使用 new/malloc，作为补偿我们传入了一段 buffer，大小为 BUFFER_SIZE = 64，你可以视情况使用。

我们的数组按照 A, B, C, buffer 的顺序在内存上连续紧密排列，且 &A = 0x30000000（这是模拟的设定，不是 A 的真实地址）

如果你需要以更自由的方式访问内存，你可以以相对 A 的方式访问，比如 A[100]，用 *(0x30000000) 是无法访问到的。

如果你有定义常量的需求（更严谨的说法是，你想定义的是汇编层面的立即数，不应该占用寄存器），请参考下面这种方式使用宏定义来完成。
*/

#include "cachelab.h"

#define m case0_m
#define n case0_n
#define p case0_p

// 我们用这个 2*2*2 的矩阵乘法来演示寄存器是怎么被分配的
void gemm_case0(ptr_reg A, ptr_reg B, ptr_reg C, ptr_reg buffer) {  // allocate 0 1 2 3
    for (reg i = 0; i < m; ++i) {                                   // allocate 4
        for (reg j = 0; j < p; ++j) {                               // allocate 5
            reg tmpc = 0;                                           // allocate 6
            for (reg k = 0; k < n; ++k) {                           // allocate 7
                reg tmpa = A[i * n + k];                            // allocate 8
                reg tmpb = B[k * p + j];                            // allocate 9
                tmpc += tmpa * tmpb;
            }  // free 9 8
            // free 7
            C[i * p + j] = tmpc;
        }  // free 6
        // free 5
    }
    // free 4
}  // free 3 2 1 0

#undef m
#undef n
#undef p

#define m case1_m
#define n case1_n
#define p case1_p

void gemm_case1(ptr_reg A, ptr_reg B, ptr_reg C, ptr_reg buffer) {
    reg block_size = 8; // 块大小
   
    for (reg ii = 0; ii < m; ii += block_size) {
        for (reg kk = 0; kk < n; kk += block_size) {
            for (reg jj = 0; jj < p; jj += block_size) {
                for (reg i = ii; i < ii + block_size; ++i) {

 // 分配临时寄存器存储部分和
					reg tmpc0 = C[i * p + (jj + 0)];
					reg tmpc1 = C[i * p + (jj + 1)];
					reg tmpc2 = C[i * p + (jj + 2)];
					reg tmpc3 = C[i * p + (jj + 3)];
					reg tmpc4 = C[i * p + (jj + 4)];
					reg tmpc5 = C[i * p + (jj + 5)];
					reg tmpc6 = C[i * p + (jj + 6)];
					reg tmpc7 = C[i * p + (jj + 7)];

					for (reg k = kk; k < kk + block_size; ++k) {
						reg tmpa = A[i * n + k];
						reg tmpb0 = B[k * p + (jj + 0)];
						reg tmpb1 = B[k * p + (jj + 1)];
						reg tmpb2 = B[k * p + (jj + 2)];
						reg tmpb3 = B[k * p + (jj + 3)];
						reg tmpb4 = B[k * p + (jj + 4)];
						reg tmpb5 = B[k * p + (jj + 5)];
						reg tmpb6 = B[k * p + (jj + 6)];
						reg tmpb7 = B[k * p + (jj + 7)];

						tmpc0 += tmpa * tmpb0;
						tmpc1 += tmpa * tmpb1;
						tmpc2 += tmpa * tmpb2;
						tmpc3 += tmpa * tmpb3;
						tmpc4 += tmpa * tmpb4;
						tmpc5 += tmpa * tmpb5;
						tmpc6 += tmpa * tmpb6;
						tmpc7 += tmpa * tmpb7;
					}
   // 写回结果
					C[i * p + (jj + 0)] = tmpc0;
					C[i * p + (jj + 1)] = tmpc1;
					C[i * p + (jj + 2)] = tmpc2;
					C[i * p + (jj + 3)] = tmpc3;
					C[i * p + (jj + 4)] = tmpc4;
					C[i * p + (jj + 5)] = tmpc5;
					C[i * p + (jj + 6)] = tmpc6;
					C[i * p + (jj + 7)] = tmpc7;
				}
			}
		}
	}
}
#undef m
#undef n
#undef p

#define m case2_m
#define n case2_n
#define p case2_p


void gemm_case2(ptr_reg A, ptr_reg B, ptr_reg C, ptr_reg buffer) {
    reg BLOCK_SIZE = 4;
    
    for (reg row_start = 0; row_start < m; row_start += BLOCK_SIZE) {
        for (reg col_start = 0; col_start < p; col_start += BLOCK_SIZE) {
            reg sum00 = 0, sum01 = 0, sum02 = 0, sum03 = 0;
            reg sum10 = 0, sum11 = 0, sum12 = 0, sum13 = 0;
            reg sum20 = 0, sum21 = 0, sum22 = 0, sum23 = 0;
            reg sum30 = 0, sum31 = 0, sum32 = 0, sum33 = 0;
            
            for (reg k = 0; k < n; ++k) {
                // 加载A的一列
                reg a0 = A[row_start * n + k];
                reg a1 = 0, a2 = 0, a3 = 0;
                
                if (row_start + 1 < m) {
                    a1 = A[(row_start + 1) * n + k];
                }
                if (row_start + 2 < m) {
                    a2 = A[(row_start + 2) * n + k];
                }
                if (row_start + 3 < m) {
                    a3 = A[(row_start + 3) * n + k];
                }
                
                // 加载B的一行
                reg b0 = B[k * p + col_start];
                reg b1 = 0, b2 = 0, b3 = 0;
                
                if (col_start + 1 < p) {
                    b1 = B[k * p + col_start + 1];
                }
                if (col_start + 2 < p) {
                    b2 = B[k * p + col_start + 2];
                }
                if (col_start + 3 < p) {
                    b3 = B[k * p + col_start + 3];
                }
                
                // 计算部分和
                sum00 += a0 * b0; sum01 += a0 * b1; sum02 += a0 * b2; sum03 += a0 * b3;
                sum10 += a1 * b0; sum11 += a1 * b1; sum12 += a1 * b2; sum13 += a1 * b3;
                sum20 += a2 * b0; sum21 += a2 * b1; sum22 += a2 * b2; sum23 += a2 * b3;
                sum30 += a3 * b0; sum31 += a3 * b1; sum32 += a3 * b2; sum33 += a3 * b3;
            }
            
            // 写回结果
            C[row_start * p + col_start] = sum00;
            if (col_start + 1 < p) C[row_start * p + col_start + 1] = sum01;
            if (col_start + 2 < p) C[row_start * p + col_start + 2] = sum02;
            if (col_start + 3 < p) C[row_start * p + col_start + 3] = sum03;
            
            if (row_start + 1 < m) {
                C[(row_start + 1) * p + col_start] = sum10;
                if (col_start + 1 < p) C[(row_start + 1) * p + col_start + 1] = sum11;
                if (col_start + 2 < p) C[(row_start + 1) * p + col_start + 2] = sum12;
                if (col_start + 3 < p) C[(row_start + 1) * p + col_start + 3] = sum13;
            }
            
            if (row_start + 2 < m) {
                C[(row_start + 2) * p + col_start] = sum20;
                if (col_start + 1 < p) C[(row_start + 2) * p + col_start + 1] = sum21;
                if (col_start + 2 < p) C[(row_start + 2) * p + col_start + 2] = sum22;
                if (col_start + 3 < p) C[(row_start + 2) * p + col_start + 3] = sum23;
            }
            
            if (row_start + 3 < m) {
                C[(row_start + 3) * p + col_start] = sum30;
                if (col_start + 1 < p) C[(row_start + 3) * p + col_start + 1] = sum31;
                if (col_start + 2 < p) C[(row_start + 3) * p + col_start + 2] = sum32;
                if (col_start + 3 < p) C[(row_start + 3) * p + col_start + 3] = sum33;
            }
        }
    }
}
#undef m
#undef n
#undef p

#define m case3_m
#define n case3_n
#define p case3_p

void gemm_case3(ptr_reg A, ptr_reg B, ptr_reg C, ptr_reg buffer) {
    reg BLOCK_SIZE = 4;
    
    for (reg row_start = 0; row_start < m; row_start += BLOCK_SIZE) {
        for (reg col_start = 0; col_start < p; col_start += BLOCK_SIZE) {
            reg sum00 = 0, sum01 = 0, sum02 = 0, sum03 = 0;
            reg sum10 = 0, sum11 = 0, sum12 = 0, sum13 = 0;
            reg sum20 = 0, sum21 = 0, sum22 = 0, sum23 = 0;
            reg sum30 = 0, sum31 = 0, sum32 = 0, sum33 = 0;
            
            for (reg k = 0; k < n; ++k) {
                // 加载A的一列
                reg a0 = A[row_start * n + k];
                reg a1 = 0, a2 = 0, a3 = 0;
                
                if (row_start + 1 < m) {
                    a1 = A[(row_start + 1) * n + k];
                }
                if (row_start + 2 < m) {
                    a2 = A[(row_start + 2) * n + k];
                }
                if (row_start + 3 < m) {
                    a3 = A[(row_start + 3) * n + k];
                }
                
                // 加载B的一行
                reg b0 = B[k * p + col_start];
                reg b1 = 0, b2 = 0, b3 = 0;
                
                if (col_start + 1 < p) {
                    b1 = B[k * p + col_start + 1];
                }
                if (col_start + 2 < p) {
                    b2 = B[k * p + col_start + 2];
                }
                if (col_start + 3 < p) {
                    b3 = B[k * p + col_start + 3];
                }
                
                // 计算部分和
                sum00 += a0 * b0; sum01 += a0 * b1; sum02 += a0 * b2; sum03 += a0 * b3;
                sum10 += a1 * b0; sum11 += a1 * b1; sum12 += a1 * b2; sum13 += a1 * b3;
                sum20 += a2 * b0; sum21 += a2 * b1; sum22 += a2 * b2; sum23 += a2 * b3;
                sum30 += a3 * b0; sum31 += a3 * b1; sum32 += a3 * b2; sum33 += a3 * b3;
            }
            
            // 写回结果
            C[row_start * p + col_start] = sum00;
            if (col_start + 1 < p) C[row_start * p + col_start + 1] = sum01;
            if (col_start + 2 < p) C[row_start * p + col_start + 2] = sum02;
            if (col_start + 3 < p) C[row_start * p + col_start + 3] = sum03;
            
            if (row_start + 1 < m) {
                C[(row_start + 1) * p + col_start] = sum10;
                if (col_start + 1 < p) C[(row_start + 1) * p + col_start + 1] = sum11;
                if (col_start + 2 < p) C[(row_start + 1) * p + col_start + 2] = sum12;
                if (col_start + 3 < p) C[(row_start + 1) * p + col_start + 3] = sum13;
            }
            
            if (row_start + 2 < m) {
                C[(row_start + 2) * p + col_start] = sum20;
                if (col_start + 1 < p) C[(row_start + 2) * p + col_start + 1] = sum21;
                if (col_start + 2 < p) C[(row_start + 2) * p + col_start + 2] = sum22;
                if (col_start + 3 < p) C[(row_start + 2) * p + col_start + 3] = sum23;
            }
            
            if (row_start + 3 < m) {
                C[(row_start + 3) * p + col_start] = sum30;
                if (col_start + 1 < p) C[(row_start + 3) * p + col_start + 1] = sum31;
                if (col_start + 2 < p) C[(row_start + 3) * p + col_start + 2] = sum32;
                if (col_start + 3 < p) C[(row_start + 3) * p + col_start + 3] = sum33;
            }
        }
    }
}

#undef m
#undef n
#undef p
