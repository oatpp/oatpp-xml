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

#include <charconv>

namespace oatpp { namespace xml {

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

oatpp::String Utils::escapeAttribute(const oatpp::String& text, char enclosingChar, data::mapping::ErrorStack& errorStack) {

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

oatpp::String Utils::unescapeAttribute(const oatpp::String& text) {
  return "";
}

oatpp::String Utils::escapeElement(const oatpp::String& text, data::mapping::ErrorStack& errorStack) {

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

}}
