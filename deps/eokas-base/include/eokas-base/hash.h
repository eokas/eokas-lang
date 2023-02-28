
#ifndef _EOKAS_BASE_HASH_H_
#define _EOKAS_BASE_HASH_H_

#include "./header.h"

namespace eokas
{
/* MD5
 * converted to C++ class by Frank Thilo (thilo@unix-ag.org)
 * for bzflag (http://www.bzflag.org)
 *
 *  based on:
 *
 *  md5.h and md5.c
 *  reference implementation of RFC 1321
 *
 *  Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
 * rights reserved.
 *
 * License to copy and use this software is granted provided that it
 * is identified as the "RSA Data Security, Inc. MD5 Message-Digest
 * Algorithm" in all material mentioning or referencing this software
 * or this function.
 *
 * License is also granted to make and use derivative works provided
 * that such works are identified as "derived from the RSA Data
 * Security, Inc. MD5 Message-Digest Algorithm" in all material
 * mentioning or referencing the derived work.
 *
 * RSA Data Security, Inc. makes no representations concerning either
 * the merchantability of this software or the suitability of this
 * software for any particular purpose. It is provided "as is"
 * without express or implied warranty of any kind.
 *
 * These notices must be retained in any copies of any part of this
 * documentation and/or software.
 */
    class MD5
    {
    public:
        static const u32_t DIGEST_SIZE = 16;

        MD5();
        String compute(const String& input);

    private:
        static const u32_t blocksize = 64;

        // low level logic operations
        static inline u32_t F(u32_t x, u32_t y, u32_t z);
        static inline u32_t G(u32_t x, u32_t y, u32_t z);
        static inline u32_t H(u32_t x, u32_t y, u32_t z);
        static inline u32_t I(u32_t x, u32_t y, u32_t z);
        static inline u32_t rotate_left(u32_t x, int n);
        static inline void FF(u32_t &a, u32_t b, u32_t c, u32_t d, u32_t x, u32_t s, u32_t ac);
        static inline void GG(u32_t &a, u32_t b, u32_t c, u32_t d, u32_t x, u32_t s, u32_t ac);
        static inline void HH(u32_t &a, u32_t b, u32_t c, u32_t d, u32_t x, u32_t s, u32_t ac);
        static inline void II(u32_t &a, u32_t b, u32_t c, u32_t d, u32_t x, u32_t s, u32_t ac);

        static void encode(u8_t* output, const u32_t* input, u32_t len);
        static void decode(u32_t* output, const u8_t* input, u32_t len);

        void init();
        void transform(const u8_t block[blocksize]);
        void update(const u8_t* buf, u32_t length);
        void finalize(u8_t* digest);

        u8_t buffer[blocksize];         // bytes that didn't fit in last 64 byte chunk
        u32_t count[2];                 // 64bit counter for number of bits (lo, hi)
        u32_t state[4];                 // digest so far
    };

    String md5(const String& input);

/*
 * Updated to C++, zedwood.com 2012
 * Based on Olivier Gay's version
 * See Modified BSD License below:
 *
 * FIPS 180-2 SHA-224/256/384/512 implementation
 * Issue date:  04/30/2005
 * http://www.ouah.org/ogay/sha2/
 *
 * Copyright (C) 2005, 2007 Olivier Gay <olivier.gay@a3.epfl.ch>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
    class SHA256
    {
    public:
        static const u32_t DIGEST_SIZE = (256 / 8);

        SHA256();
        String compute(const String& input);

    private:
        static const u32_t sha256_k[];
        static const u32_t SHA224_256_BLOCK_SIZE = (512 / 8);

        void init();
        void transform(const u8_t* message, u32_t block_nb);
        void update(const u8_t* message, u32_t len);
        void finalize(u8_t* digest);

        u32_t m_tot_len;
        u32_t m_len;
        u8_t m_block[2 * SHA224_256_BLOCK_SIZE];
        u32_t m_h[8];
    };

    String sha256(const String& input);
}

#endif //_EOKAS_BASE_HASH_H_
