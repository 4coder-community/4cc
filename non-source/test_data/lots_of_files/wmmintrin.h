/***
*** Copyright (C) 1985-2008 Intel Corporation.  All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
***
****/

/*
 * wmmintrin.h
 *
 * Principal header file for Intel(R) AES and PCLMULQDQ intrinsics.
 */

#pragma once
#ifndef __midl
#ifndef _INCLUDED_WMM
#define _INCLUDED_WMM

#if defined (_M_CEE_PURE)
        #error ERROR: EMM intrinsics not supported in the pure mode!
#else  /* defined (_M_CEE_PURE) */

#include <nmmintrin.h>


#if __cplusplus
extern "C" {
#endif  /* __cplusplus */

/*
 * Performs 1 round of AES decryption of the first m128i using
 * the second m128i as a round key.
 */
extern __m128i _mm_aesdec_si128(__m128i v, __m128i rkey);

/*
 * Performs the last round of AES decryption of the first m128i
 * using the second m128i as a round key.
 */
extern __m128i _mm_aesdeclast_si128(__m128i v, __m128i rkey);

/*
 * Performs 1 round of AES encryption of the first m128i using
 * the second m128i as a round key.
 */
extern __m128i _mm_aesenc_si128(__m128i v, __m128i rkey);

/*
 * Performs the last round of AES encryption of the first m128i
 * using the second m128i as a round key.
 */
extern __m128i _mm_aesenclast_si128(__m128i v, __m128i rkey);

/*
 * Performs the InverseMixColumn operation on the source m128i
 * and stores the result into m128i destination.
 */
extern __m128i _mm_aesimc_si128(__m128i v);

/*
 * Generates a m128i round key for the input m128i
 * AES cipher key and byte round constant.
 * The second parameter must be a compile time constant.
 */
extern __m128i _mm_aeskeygenassist_si128(__m128i ckey, const int rcon);

/*
 * Performs carry-less integer multiplication of 64-bit halves
 * of 128-bit input operands.
 * The third parameter inducates which 64-bit haves of the input parameters
 * v1 and v2 should be used. It must be a compile time constant.
 */
extern __m128i _mm_clmulepi64_si128(__m128i v1, __m128i v2,
                                            const int imm8);




#if defined __cplusplus
}; /* End "C" */
#endif  /* defined __cplusplus */

#endif  /* defined (_M_CEE_PURE) */

#endif  /* _INCLUDED_WMM */
#endif  /* __midl */
