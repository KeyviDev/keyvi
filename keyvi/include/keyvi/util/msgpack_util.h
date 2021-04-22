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

#ifndef KEYVI_UTIL_MSGPACK_UTIL_H_
#define KEYVI_UTIL_MSGPACK_UTIL_H_
#include <limits>

#include "msgpack.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"

/**
 * Utility classes for msgpack.
 *
 * In particular this implements a way to decide at runtime to use single precision floats when turning json into
 * msgpack. The trick is to write to use a buffer implementation which name is used for a template specialization
 * which does the trick.
 */

namespace keyvi {
namespace util {

inline void JsonToMsgPack(const rapidjson::Value& value, msgpack::packer<msgpack::sbuffer>* msgpack_packer,
                          bool single_precision_float) {
  switch (value.GetType()) {
    case rapidjson::kArrayType:
      msgpack_packer->pack_array(value.Size());
      for (auto& v : value.GetArray()) {
        JsonToMsgPack(v, msgpack_packer, single_precision_float);
      }
      break;
    case rapidjson::kObjectType:
      msgpack_packer->pack_map(value.MemberCount());
      for (auto& v : value.GetObj()) {
        msgpack_packer->pack_str(v.name.GetStringLength());
        msgpack_packer->pack_str_body(v.name.GetString(), v.name.GetStringLength());
        JsonToMsgPack(v.value, msgpack_packer, single_precision_float);
      }
      break;
    case rapidjson::kNumberType:
      if (value.IsDouble()) {
        if (single_precision_float == false || value.GetDouble() < std::numeric_limits<float>::min() ||
            value.GetDouble() > std::numeric_limits<float>::max()) {
          msgpack_packer->pack_double(value.GetDouble());
        } else {
          msgpack_packer->pack_float(value.GetFloat());
        }
      } else if (value.IsUint64()) {
        msgpack_packer->pack_uint64(value.GetUint64());
      } else {
        msgpack_packer->pack_int64(value.GetInt64());
      }
      break;
    case rapidjson::kStringType:
      msgpack_packer->pack_str(value.GetStringLength());
      msgpack_packer->pack_str_body(value.GetString(), value.GetStringLength());
      break;
    case rapidjson::kFalseType:
      msgpack_packer->pack_false();
      break;
    case rapidjson::kTrueType:
      msgpack_packer->pack_true();
      break;
    case rapidjson::kNullType:
      msgpack_packer->pack_nil();
      break;
  }
}

template <typename Writer>
inline void MsgPackDump(Writer* writer, const msgpack::object& o) {
  switch (o.type) {
    case msgpack::type::NIL:
      writer->Null();
      break;

    case msgpack::type::BOOLEAN:
      writer->Bool(o.via.boolean);
      break;

    case msgpack::type::POSITIVE_INTEGER:
      writer->Uint64(o.via.u64);
      break;

    case msgpack::type::NEGATIVE_INTEGER:
      writer->Int64(o.via.i64);
      break;

    case msgpack::type::FLOAT32:
    case msgpack::type::FLOAT64:
      writer->Double(o.via.f64);
      break;

    case msgpack::type::STR:
      writer->String(o.via.str.ptr, o.via.str.size);
      break;

    case msgpack::type::ARRAY:
      writer->StartArray();
      {
        const msgpack::object* pend_array(o.via.array.ptr + o.via.array.size);
        for (msgpack::object* p(o.via.array.ptr); p < pend_array; ++p) {
          MsgPackDump(writer, *p);
        }
      }
      writer->EndArray();
      break;

    case msgpack::type::MAP:
      writer->StartObject();
      {
        const msgpack::object_kv* pend_map(o.via.map.ptr + o.via.map.size);
        for (msgpack::object_kv* p(o.via.map.ptr); p < pend_map; ++p) {
          writer->Key(p->key.via.str.ptr, p->key.via.str.size);
          MsgPackDump(writer, p->val);
        }
      }
      writer->EndObject();
      break;

    // there is no counterpart for binary and ext in json, this branch should never be executed
    case msgpack::type::BIN:
    case msgpack::type::EXT:
    default:
      break;
  }
}

} /* namespace util */
} /* namespace keyvi */

#endif  // KEYVI_UTIL_MSGPACK_UTIL_H_
