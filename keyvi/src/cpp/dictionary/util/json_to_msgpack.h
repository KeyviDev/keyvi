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
 * json_to_msgpack.h
 *
 * conversion between (rapid)json and msgpack, based on:
 * https://github.com/xpol/xchange/blob/master/src/msgpack/type/rapidjson/document.hpp
 *  Created on: Nov 3, 2014
 *      Author: hendrik
 */

#ifndef JSON_TO_MSGPACK_H_
#define JSON_TO_MSGPACK_H_

#include <msgpack_fwd.hpp>
#include <rapidjson/document.h>

//#define ENABLE_TRACING
#include "dictionary/util/trace.h"

// needs to be in the msgpack namespace in order to resolve
namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(v1) {

  template <typename Encoding, typename Allocator, typename StackAllocator>
  inline rapidjson::GenericDocument<Encoding, Allocator, StackAllocator>& operator>> (const object& o, rapidjson::GenericDocument<Encoding, Allocator, StackAllocator>& v)
  {
    switch (o.type)
    {
    case msgpack::type::BOOLEAN: v.SetBool(o.via.boolean); break;
    case msgpack::type::POSITIVE_INTEGER: v.SetUint64(o.via.u64); break;
    case msgpack::type::NEGATIVE_INTEGER: v.SetInt64(o.via.i64); break;
    case msgpack::type::DOUBLE: v.SetDouble(o.via.dec); break;
    case msgpack::type::STR: v.SetString(o.via.str.ptr, o.via.str.size); break;
    case msgpack::type::ARRAY:{
      v.SetArray();
      v.Reserve(o.via.array.size, v.GetAllocator());
      msgpack::object* ptr = o.via.array.ptr;
      msgpack::object* END = ptr + o.via.array.size;
      for (; ptr < END; ++ptr)
      {
        rapidjson::GenericDocument<Encoding, Allocator, StackAllocator> element(&v.GetAllocator());
        ptr->convert(&element);
        v.PushBack(static_cast<rapidjson::GenericValue<Encoding, Allocator>&>(element), v.GetAllocator());
      }
    }
      break;
    case msgpack::type::MAP: {
      v.SetObject();
      msgpack::object_kv* ptr = o.via.map.ptr;
      msgpack::object_kv* END = ptr + o.via.map.size;
      for (; ptr < END; ++ptr)
      {
                rapidjson::GenericValue<Encoding, Allocator> key(ptr->key.via.str.ptr, ptr->key.via.str.size, v.GetAllocator());
        rapidjson::GenericDocument<Encoding, Allocator, StackAllocator> val(&v.GetAllocator());
        ptr->val.convert(&val);

                v.AddMember(key, val, v.GetAllocator());
      }
    }
      break;
    case msgpack::type::NIL:
    default:
      v.SetNull(); break;

    }
    return v;
  }

  template <typename Encoding, typename Allocator>
  inline rapidjson::GenericValue<Encoding, Allocator>& operator>> (const object& o, rapidjson::GenericValue<Encoding, Allocator>& v)
  {
    rapidjson::GenericDocument<Encoding, Allocator> d;
    o >> d;
    return v = d;
  }

  template <typename Stream, typename Encoding, typename Allocator>
  inline packer<Stream>& operator<< (packer<Stream>& o, const rapidjson::GenericValue<Encoding, Allocator>& v)
  {
        switch (v.GetType())
        {
            case rapidjson::kNullType:
                return o.pack_nil();
            case rapidjson::kFalseType:
                return o.pack_false();
            case rapidjson::kTrueType:
                return o.pack_true();
            case rapidjson::kObjectType:
            {
                o.pack_map(v.MemberCount());
                typename rapidjson::GenericValue<Encoding, Allocator>::ConstMemberIterator i = v.MemberBegin(), END = v.MemberEnd();
                for (; i != END; ++i)
                {
                    o.pack_str(i->name.GetStringLength()).pack_str_body(i->name.GetString(), i->name.GetStringLength());
                    o.pack(i->value);
                }
                return o;
            }
            case rapidjson::kArrayType:
            {
                o.pack_array(v.Size());
                typename rapidjson::GenericValue<Encoding, Allocator>::ConstValueIterator i = v.Begin(), END = v.End();
                for (;i < END; ++i)
                    o.pack(*i);
                return o;
            }
            case rapidjson::kStringType:
                return o.pack_str(v.GetStringLength()).pack_str_body(v.GetString(), v.GetStringLength());
            case rapidjson::kNumberType:
                if (v.IsInt())
                    return o.pack_int(v.GetInt());
                if (v.IsUint())
                    return o.pack_unsigned_int(v.GetUint());
                if (v.IsInt64())
                    return o.pack_int64(v.GetUint64());
                if (v.IsUint64())
                    return o.pack_uint64(v.GetUint64());
                if (v.IsDouble()||v.IsNumber())
                    return o.pack_double(v.GetDouble());
            default:
                return o;
        }
  }

  template <typename Stream, typename Encoding, typename Allocator>
  inline packer<Stream>& operator<< (packer<Stream>& o, const rapidjson::GenericDocument<Encoding, Allocator>& v)
  {
    o << static_cast<const rapidjson::GenericValue<Encoding, Allocator>&>(v);
    return o;
  }

  template <typename Encoding, typename Allocator>
  inline void operator<< (object::with_zone& o, rapidjson::GenericValue<Encoding, Allocator>& v)
  {
        switch (v.GetType())
        {
            case rapidjson::kNullType:
                o.type = type::NIL;
                break;
            case rapidjson::kFalseType:
                o.type = type::BOOLEAN;
                o.via.boolean = false;
                break;
            case rapidjson::kTrueType:
                o.type = type::BOOLEAN;
                o.via.boolean = true;
                break;
            case rapidjson::kObjectType:
            {
                o.type = type::MAP;
                if (v.ObjectEmpty()) {
                    o.via.map.ptr = NULL;
                    o.via.map.size = 0;
                }
                else {
                    size_t sz = v.MemberCount();
                    object_kv* p = (object_kv*)o.zone.allocate_align(sizeof(object_kv)*sz);
                    object_kv* const pend = p + sz;
                    o.via.map.ptr = p;
                    o.via.map.size = sz;
                    typename rapidjson::GenericValue<Encoding, Allocator>::ConstMemberIterator it(v.MemberBegin());
                    do {
                        p->key = object(it->name, o.zone);
                        p->val = object(it->value, o.zone);
                        ++p;
                        ++it;
                    } while (p < pend);
                }
                break;
            }
            case rapidjson::kArrayType:
            {
                o.type = type::ARRAY;
                if (v.Empty()) {
                    o.via.array.ptr = NULL;
                    o.via.array.size = 0;
                }
                else {
                    object* p = (object*)o.zone.allocate_align(sizeof(object)*v.Size());
                    object* const pend = p + v.Size();
                    o.via.array.ptr = p;
                    o.via.array.size = v.Size();
                    typename rapidjson::GenericValue<Encoding, Allocator>::ConstValueIterator it(v.Begin());
                    do {
                        *p = object(*it, o.zone);
                        ++p;
                        ++it;
                    } while (p < pend);
                }
                break;
            }
            case rapidjson::kStringType:
            {
                o.type = type::STR;
                std::string s(v.GetString(), v.GetStringLength());

                char* ptr = (char*)o.zone.allocate_align(s.size());
                o.via.str.ptr = ptr;
                o.via.str.size = (uint32_t)s.size();
                memcpy(ptr, s.data(), s.size());
                break;
            }
            case rapidjson::kNumberType:
                if (v.IsInt())
                {
                    o.type = type::NEGATIVE_INTEGER;
                    o.via.i64 = v.GetInt();
                }
                else if (v.IsUint())
                {
                    o.type = type::POSITIVE_INTEGER;
                    o.via.u64 = v.GetUint();
                }
                else if (v.IsInt64())
                {
                    o.type = type::NEGATIVE_INTEGER;
                    o.via.i64 = v.GetInt64();
                }
                else if (v.IsUint64())
                {
                    o.type = type::POSITIVE_INTEGER;
                    o.via.u64 = v.GetUint64();
                }
                else if (v.IsDouble())
                {
                    o.type = type::DOUBLE;
                    o.via.dec = v.GetDouble();
                }
                break;
            default:
                break;

    }
  }


  template <typename Encoding, typename Allocator, typename StackAllocator>
  inline void operator<< (object::with_zone& o, rapidjson::GenericDocument<Encoding, Allocator, StackAllocator>& v)
  {
    o << static_cast<rapidjson::GenericValue<Encoding, Allocator>&>(v);
  }

} // MSGPACK_API_VERSION_NAMESPACE(v1)
} // namespace msgpack


#endif /* JSON_TO_MSGPACK_H_ */
