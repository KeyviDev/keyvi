/* * keyvi - A key value store.
 *
 * Copyright 2015 Hendrik Muhs<hendrik.muhs@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * endian.h
 *
 *  Created on: Apr 28, 2015
 *      Author: hendrik
 */

#ifndef SRC_CPP_DICTIONARY_UTIL_ENDIAN_H_
#define SRC_CPP_DICTIONARY_UTIL_ENDIAN_H_

#if defined(OS_MACOSX)
    #include <libkern/OSByteOrder.h>

    #define htobe16(x) OSSwapHostToBigInt16(x)
    #define htole16(x) OSSwapHostToLittleInt16(x)
    #define be16toh(x) OSSwapBigToHostInt16(x)
    #define le16toh(x) OSSwapLittleToHostInt16(x)

    #define htobe32(x) OSSwapHostToBigInt32(x)
    #define htole32(x) OSSwapHostToLittleInt32(x)
    #define be32toh(x) OSSwapBigToHostInt32(x)
    #define le32toh(x) OSSwapLittleToHostInt32(x)

    #define htobe64(x) OSSwapHostToBigInt64(x)
    #define htole64(x) OSSwapHostToLittleInt64(x)
    #define be64toh(x) OSSwapBigToHostInt64(x)
    #define le64toh(x) OSSwapLittleToHostInt64(x)

    #if defined(__LITTLE_ENDIAN__)
        #define CLQ_LITTLE_ENDIAN
    #elif defined(__BIG_ENDIAN__)
        #define CLQ_BIG_ENDIAN
    #else
        #error "Unknown endianess"
    #endif

#elif defined(OS_SOLARIS)
    #include <sys/isa_defs.h>
    #ifdef _LITTLE_ENDIAN
        #define LITTLE_ENDIAN
    #else
        #define BIG_ENDIAN
    #endif
#elif defined(OS_FREEBSD) || defined(OS_OPENBSD) || defined(OS_NETBSD) || defined(OS_DRAGONFLYBSD)
    #include <sys/types.h>
    #include <sys/endian.h>
#elif defined(__linux__) && (__BYTE_ORDER == __LITTLE_ENDIAN) && (__GLIBC__ <= 2 && __GLIBC_MINOR__ < 9)

    #define CLQ_LITTLE_ENDIAN
    #define htole16(x) (x)
    #define le16toh(x) (x)
    #define htobe32(x) __bswap_32(x)
    #define be32toh(x) __bswap_32(x)

#else
    #include <endian.h>
    #if __BYTE_ORDER == __LITTLE_ENDIAN
        #define CLQ_LITTLE_ENDIAN
    #elif __BYTE_ORDER == __BIG_ENDIAN
        #define CLQ_BIG_ENDIAN
    #else
        #error "Unknown endianess"
    #endif

#endif



#endif /* SRC_CPP_DICTIONARY_UTIL_ENDIAN_H_ */
