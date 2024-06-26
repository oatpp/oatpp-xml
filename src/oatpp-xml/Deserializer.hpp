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

#ifndef OATPP_XML_DESERIALIZER_HPP
#define OATPP_XML_DESERIALIZER_HPP

#include "./Utils.hpp"

#include "oatpp/data/mapping/ObjectMapper.hpp"
#include "oatpp/data/mapping/Tree.hpp"

#include "oatpp/utils/parser/Caret.hpp"
#include "oatpp/Types.hpp"

namespace oatpp { namespace xml {

class Deserializer {
public:

  /**
   * Deserializer config.
   */
  class Config : public oatpp::base::Countable {
  public:

  };

public:

  struct State {
    const Config* config;
    data::mapping::Tree* tree;
    utils::parser::Caret* caret;
    data::mapping::ErrorStack errorStack;
  };

public:

  static oatpp::String parseElementName(State& state);
  static oatpp::String parseAttributeName(State& state);
  static oatpp::String parseAttributeValue(State& state);
  static void parseAttributes(State& state);

  static void parsePINode(State& state, oatpp::String& name);
  static void parseCommentNode(State& state, oatpp::String& name);
  static void parseCDataNode(State& state, oatpp::String& name);
  static void parseElementContent(State& state, const oatpp::String& name);
  static void parseElementNode(State& state, oatpp::String& name);
  static void parseNode(State& state, oatpp::String& name);

public:

  static void deserialize(State& state);

};

}}

#endif /* OATPP_XML_DESERIALIZER_HPP */
