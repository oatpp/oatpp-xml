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

#ifndef OATPP_XML_UTILS_HPP
#define OATPP_XML_UTILS_HPP

#include "oatpp/data/mapping/ObjectMapper.hpp"
#include "oatpp/data/stream/Stream.hpp"

namespace oatpp { namespace xml {

class Utils {
public:

  static v_uint32 escapeChar(data::stream::ConsistentOutputStream* stream,
                             const char* buffer, v_buff_usize bufferSize,
                             data::mapping::ErrorStack& errorStack);

  static oatpp::String escapeAttribute(const oatpp::String& text, char enclosingChar, data::mapping::ErrorStack& errorStack);
  static oatpp::String unescapeAttribute(const oatpp::String& text);

  static oatpp::String escapeElement(const oatpp::String& text, data::mapping::ErrorStack& errorStack);

};

}}

#endif //OATPP_XML_UTILS_HPP
