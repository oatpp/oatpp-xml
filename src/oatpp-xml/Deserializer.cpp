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

#include "Deserializer.hpp"

namespace oatpp { namespace xml {

oatpp::String Deserializer::parseElementName(State& state) {
  auto data = state.caret->getCurrData();
  auto size = state.caret->getDataSize() - state.caret->getPosition();
  for(v_buff_size i = 0; i < size; i ++) {
    auto c = data[i];
    if(i > 0 && (c == '/' || c == '>' || c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f')) {
      state.caret->inc(i);
      return oatpp::String(data, i);
    }
    bool validChar = c >= 'a' && c <= 'z' ||
                     c >= 'A' && c <= 'Z' ||
                     c >= '0' && c <= '9' ||
                     c == ':' || c == '.' || c == '_' || c == '-' ||
                     c > 127;
    if(!validChar) {
      state.errorStack.push("[oatpp::xml::Deserializer::parseElementName()]: Invalid element name '" + oatpp::String(data, i + 1) + "'");
      return nullptr;
    }
  }
  state.errorStack.push("[oatpp::xml::Deserializer::parseElementName()]: Invalid element name");
  return nullptr;
}

oatpp::String Deserializer::parseAttributeName(State& state) {
  auto data = state.caret->getCurrData();
  auto size = state.caret->getDataSize() - state.caret->getPosition();
  for(v_buff_size i = 0; i < size; i ++) {
    auto c = data[i];
    if(i > 0 && (c == '=' || c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f')) {
      state.caret->inc(i);
      return oatpp::String(data, i);
    }
    bool validChar = c >= 'a' && c <= 'z' ||
                     c >= 'A' && c <= 'Z' ||
                     c >= '0' && c <= '9' ||
                     c == ':' || c == '.' || c == '_' || c == '-' ||
                     c > 127;
    if(!validChar) {
      state.errorStack.push("[oatpp::xml::Deserializer::parseAttributeName()]: Invalid attribute name '" + oatpp::String(data, i + 1) + "'");
      return nullptr;
    }
  }
  state.errorStack.push("[oatpp::xml::Deserializer::parseAttributeName()]: Invalid attribute name");
  return nullptr;
}

oatpp::String Deserializer::parseAttributeValue(State& state) {

  char enclosingChar;

  if(state.caret->isAtChar('\'')) {
    enclosingChar = '\'';
  } else if(state.caret->isAtChar('"')) {
    enclosingChar = '"';
  } else {
    state.errorStack.push(R"([oatpp::xml::Deserializer::parseElementName()]: "'" or '"' is missing)");
    return nullptr;
  }

  state.caret->inc(1);

  auto label = state.caret->putLabel();

  if(!state.caret->findChar(enclosingChar)) {
    state.errorStack.push(R"([oatpp::xml::Deserializer::parseElementName()]: Unterminated attribute value)");
    return nullptr;
  }

  auto result = Utils::unescapeText(label.toString(), state.errorStack);
  state.caret->inc(1);
  return result;

}

void Deserializer::parseAttributes(State& state) {

  auto& caret = state.caret;
  while(caret->canContinue()) {

    caret->skipBlankChars();
    if(caret->isAtChar('/') || caret->isAtChar('>')) {
      return;
    }

    auto key = parseAttributeName(state);
    if(!state.errorStack.empty()) {
      state.errorStack.push("[oatpp::xml::Deserializer::parseAttributes()]");
      return;
    }

    caret->skipBlankChars();
    if(!caret->canContinueAtChar('=', 1)) {
      state.errorStack.push("[oatpp::xml::Deserializer::parseAttributes()]: '=' is missing for '" + key + "'");
      return;
    }

    caret->skipBlankChars();
    auto value = parseAttributeValue(state);

    if(!state.errorStack.empty()) {
      state.errorStack.push("[oatpp::xml::Deserializer::parseAttributes()]: key='" + key + "'");
      return;
    }

    state.tree->attributes()[key] = value;

  }

}

void Deserializer::deserialize(oatpp::xml::Deserializer::State &state) {

}


}}
