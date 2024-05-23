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


#ifndef OATPP_XML_SERIALIZER_HPP
#define OATPP_XML_SERIALIZER_HPP

#include "./Utils.hpp"

#include "oatpp/data/mapping/ObjectMapper.hpp"
#include "oatpp/data/mapping/Tree.hpp"
#include "oatpp/Types.hpp"

namespace oatpp { namespace xml {

class Serializer {
public:
  /**
   * Serializer config.
   */
  class Config : public oatpp::base::Countable {
  public:

    /**
     * Include fields with value == nullptr into serialized XML.
     */
    bool includeNullElements = true;

    /**
     * Use XML Beautifier.
     */
    bool useBeautifier = false;

    /**
     * Beautifier Indent.
     */
    oatpp::String beautifierIndent = "  ";

    /**
     * Beautifier new line.
     */
    oatpp::String beautifierNewLine = "\n";

    /**
     * Escape flags.
     */
    v_uint32 escapeFlags = xml::Utils::FLAG_ESCAPE_STANDARD_ONLY;
  };

public:

  struct State {

    const Config* config;
    const data::mapping::Tree* tree;
    data::stream::ConsistentOutputStream* stream;

    data::mapping::ErrorStack errorStack;

  };

private:
  static void startNode(const oatpp::String& name, State& state);
  static void endNode(const oatpp::String& name, State& state);
private:

  static void serializeString(State& state);
  static void serializeArray(State& state);
  static void serializeMap(State& state);
  static void serializePairs(State& state);

public:

  static void serialize(State& state);

};

}}

#endif /* OATPP_XML_SERIALIZER_HPP */
