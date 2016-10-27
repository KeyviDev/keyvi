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
 * msgpack_util.h
 *
 *  Created on: Oct 27, 2016
 *      Author: hendrik
 */

#ifndef UTIL_MSGPACK_UTIL_H_
#define UTIL_MSGPACK_UTIL_H_

#include "msgpack.hpp"

/**
 * Utility classes for msgpack.
 *
 * In particular this implements a way to decide at runtime to use single precision floats when turning json into
 * msgpack. The trick is to write to use a buffer implementation which name is used for a template specialization
 * which does the trick.
 */

namespace keyvi {
namespace dictionary {
namespace util {

class msgpack_buffer: public msgpack::sbuffer {
 public:
  using msgpack::sbuffer::sbuffer;

  void set_single_precision_float() {
    single_precision_float_ = true;
  }

  bool get_single_precision_float() const {
    return single_precision_float_;
  }

 private:
  bool single_precision_float_ = false;
};

} /* namespace util */
} /* namespace dictionary */
} /* namespace keyvi */

namespace msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
/// @endcond


/**
 * Specialization to turn doubles into msgpack single or double precision floats based on the setting in the buffer.
 *
 * @param d the double to pack
 * @return the packer instance
 */

template <>
inline packer<keyvi::dictionary::util::msgpack_buffer>& packer<keyvi::dictionary::util::msgpack_buffer>::pack_double(double d)
{
  // taken from msgpack code
  if (m_stream.get_single_precision_float()) {
    union { float f; uint32_t i; } mem;
    mem.f = d;
    char buf[5];
    buf[0] = static_cast<char>(0xcau); _msgpack_store32(&buf[1], mem.i);
    append_buffer(buf, 5);
  } else {
    union { double f; uint64_t i; } mem;
    mem.f = d;
    char buf[9];
    buf[0] = static_cast<char>(0xcbu);

#if defined(TARGET_OS_IPHONE)
    // ok
#elif defined(__arm__) && !(__ARM_EABI__) // arm-oabi
    // https://github.com/msgpack/msgpack-perl/pull/1
    mem.i = (mem.i & 0xFFFFFFFFUL) << 32UL | (mem.i >> 32UL);
#endif
    _msgpack_store64(&buf[1], mem.i);
    append_buffer(buf, 9);
  }

  return *this;
}

}
}

#endif /* UTIL_MSGPACK_UTIL_H_ */
