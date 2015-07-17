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
 * json_to_msgpack_fwd.h
 *
 *  Created on: Nov 3, 2014
 *      Author: hendrik
 */

#ifndef JSON_TO_MSGPACK_FWD_H_
#define JSON_TO_MSGPACK_FWD_H_

#include <msgpack_fwd.hpp>
#include <rapidjson/document.h>

// needs to be in the msgpack namespace in order to resolve
namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(v1) {

  template <typename Encoding, typename Allocator, typename StackAllocator>
  inline rapidjson::GenericDocument<Encoding, Allocator, StackAllocator>& operator>> (const object& o, rapidjson::GenericDocument<Encoding, Allocator, StackAllocator>& v);

  template <typename Encoding, typename Allocator>
  inline rapidjson::GenericValue<Encoding, Allocator>& operator>> (const object& o, rapidjson::GenericValue<Encoding, Allocator>& v);

  template <typename Stream, typename Encoding, typename Allocator>
  inline packer<Stream>& operator<< (packer<Stream>& o, const rapidjson::GenericValue<Encoding, Allocator>& v);

  template <typename Stream, typename Encoding, typename Allocator>
  inline packer<Stream>& operator<< (packer<Stream>& o, const rapidjson::GenericDocument<Encoding, Allocator>& v);

  template <typename Encoding, typename Allocator>
  inline void operator<< (object::with_zone& o, rapidjson::GenericValue<Encoding, Allocator>& v);

  template <typename Encoding, typename Allocator, typename StackAllocator>
  inline void operator<< (object::with_zone& o, rapidjson::GenericDocument<Encoding, Allocator, StackAllocator>& v);

} // MSGPACK_API_VERSION_NAMESPACE(v1)
} // namespace msgpack


#endif /* JSON_TO_MSGPACK_FWD_H_ */
