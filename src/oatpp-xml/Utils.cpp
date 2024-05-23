/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/


#include "Utils.hpp"

#include "oatpp/data/stream/BufferStream.hpp"
#include "oatpp/encoding/Unicode.hpp"
#include "oatpp/utils/Conversion.hpp"

#include <cstdlib>

namespace oatpp { namespace xml {

const std::unordered_map<data::share::StringKeyLabel, std::string> Utils::PREDEFINED_ENTITIES = {
  {"&amp;", "&"},
  {"&lt;", "<"},
  {"&gt;", ">"},
  {"&quot;", "\""},
  {"&apos;", "'"}
};

v_uint32 Utils::escapeChar(data::stream::ConsistentOutputStream* stream,
                           const char * buffer, v_buff_usize bufferSize,
                           data::mapping::ErrorStack& errorStack)
{
  auto c = static_cast<v_char8>(*buffer);
  switch (c) {
    case '&': stream->writeSimple("&amp;", 5); return 1;
    case '<': stream->writeSimple("&lt;", 4); return 1;
    case '>': stream->writeSimple("&gt;", 4); return 1;
    default:
      if(c >=32 && c < 128) {
        stream->writeSimple(buffer, 1);
        return 1;
      }
      auto charLength = encoding::Unicode::getUtf8CharSequenceLength(c);
      if(bufferSize < charLength) {
        errorStack.push("[oatpp::xml::Utils::escapeChar()]: Invalid character");
        return 0;
      }
      auto code = encoding::Unicode::encodeUtf8Char(buffer, charLength);

//      char hexValue[32] = {0};
//      std::to_chars(hexValue, hexValue + 32, code, 16);
//
//      stream->writeSimple("&#x", 3);
//      stream->writeSimple(hexValue);
//      stream->writeSimple(";", 1);

      stream->writeSimple("&#", 2);
      stream->writeAsString(code);
      stream->writeSimple(";", 1);
      return static_cast<v_uint32>(charLength);
  }
}

bool Utils::unescapeChar(data::stream::ConsistentOutputStream* stream, const data::share::StringKeyLabel& charRef) {

  auto data = static_cast<const v_char8*>(charRef.getData());
  auto dataSize = charRef.getSize();

  if(dataSize > 3 && data[1] == '#' && data[dataSize - 1] == ';') {

    v_uint32 code;

    if(data[2] == 'x') {

      if(dataSize < 5) {
        return false;
      }
      for(v_buff_usize i = 3; i < dataSize - 1; i++) {
        auto c = data[i];
        bool validChar = (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f') || (c >= '0' && c <= '9');
        if(!validChar) return false;
      }

      code = std::strtoul(reinterpret_cast<const char *>(data + 3), nullptr, 16);

    } else {

      for(v_buff_usize i = 2; i < dataSize - 1; i++) {
        auto c = data[i];
        bool validChar = c >= '0' && c <= '9';
        if(!validChar) return false;
      }

      code = std::strtoul(reinterpret_cast<const char *>(data + 2), nullptr, 10);

    }

    v_char8 buff[32];
    auto size = encoding::Unicode::decodeUtf8Char(static_cast<v_int32>(code), buff);
    stream->writeSimple(buff, size);
    return true;

  }

  auto it = PREDEFINED_ENTITIES.find(charRef);
  if(it != PREDEFINED_ENTITIES.end()) {
    auto& value = it->second;
    stream->writeSimple(value.data(), static_cast<v_buff_size>(value.size()));
    return true;
  }

  /* if not found - write as-is */
  stream->writeSimple(charRef.getData(), charRef.getSize());
  return false;

}

oatpp::String Utils::escapeAttributeText(const oatpp::String& text, char enclosingChar, data::mapping::ErrorStack& errorStack) {

  if(text == nullptr) {
    return "";
  }

  data::stream::BufferOutputStream ss(256);

  auto data = text->data();

  v_buff_usize i = 0;
  while(i < text->size()) {

    auto c = data[i];

    switch (c) {
      case '"': {
        if(enclosingChar == '"') {
          ss.writeSimple("&quot;", 6);
        } else if(enclosingChar == '\'') {
          ss.writeSimple("\"", 1);
        }
        i ++;
        break;
      }
      case '\'': {
        if(enclosingChar == '\'') {
          ss.writeSimple("&apos;", 6);
        } else if(enclosingChar == '"') {
          ss.writeSimple("'", 1);
        }
        i ++;
        break;
      }
      default:
        i += escapeChar(&ss, &data[i], text->size() - i, errorStack);
        if(!errorStack.empty()) {
          return "";
        }
        break;
    }
  }

  return ss.toString();

}

oatpp::String Utils::escapeElementText(const oatpp::String& text, data::mapping::ErrorStack& errorStack) {

  if(text == nullptr) {
    return "";
  }

  data::stream::BufferOutputStream ss(256);

  auto data = text->data();

  v_buff_usize i = 0;
  while(i < text->size()) {
    i += escapeChar(&ss, &data[i], text->size() - i, errorStack);
    if(!errorStack.empty()) {
      return "";
    }
  }

  return ss.toString();

}

oatpp::String Utils::unescapeText(const oatpp::String& text, data::mapping::ErrorStack& errorStack) {

  data::stream::BufferOutputStream ss(256);

  utils::parser::Caret caret(text);
  while (caret.canContinue()) {
    auto label = caret.putLabel();
    auto found = caret.findChar('&');
    ss.writeSimple(label.getData(), label.getSize());
    if(found) {
      auto data = reinterpret_cast<const v_char8*>(caret.getCurrData());
      auto size = caret.getDataSize() - caret.getPosition();
      v_buff_size i;
      for(i = 1; i < size; i ++) {
        auto c = data[static_cast<v_buff_usize>(i)];
        bool validChar = (c >= 'A' && c <= 'Z') ||
                         (c >= 'a' && c <= 'z') ||
                         (c >= '0' && c <= '9') ||
                          c == '#' || c == 'x';
        if(!validChar) {
          if(c == ';') {
            i ++;
          }
          break;
        }
      }
      unescapeChar(&ss, data::share::StringKeyLabel(nullptr, reinterpret_cast<const char*>(data), i));
      caret.inc(i);
    }
  }

  return ss.toString();

}

}}
