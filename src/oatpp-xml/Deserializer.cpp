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

void Deserializer::parsePINode(State& state, oatpp::String& name) {

  if(!state.caret->isAtText("<?", 2, true)) {
    state.errorStack.push("[oatpp::xml::Deserializer::parsePINode()]: '<?' expected");
    return;
  }

  name = "?" + parseElementName(state);
  if(!state.errorStack.empty()) {
    state.errorStack.push("[oatpp::xml::Deserializer::parsePINode()]");
    return;
  }

  auto label = state.caret->putLabel();

  if(!state.caret->findText("?>", 2)) {
    state.errorStack.push("[oatpp::xml::Deserializer::parsePINode()]: unterminated PI node");
    return;
  }

  state.tree->setString(label.toString());
  state.caret->inc(2);

}

void Deserializer::parseCommentNode(State& state, oatpp::String& name) {

  if(!state.caret->isAtText("<!--", 4, true)) {
    state.errorStack.push("[oatpp::xml::Deserializer::parseCommentNode()]: '<!--' expected");
    return;
  }

  auto label = state.caret->putLabel();

  if(!state.caret->findText("-->", 3)) {
    state.errorStack.push("[oatpp::xml::Deserializer::parseCommentNode()]: unterminated comment");
    return;
  }

  state.tree->setString(label.toString());
  state.caret->inc(3);
  name = "!COMMENT";

}

void Deserializer::parseCDataNode(State& state, oatpp::String& name) {
  if(!state.caret->isAtText("<![CDATA[", 9, true)) {
    state.errorStack.push("[oatpp::xml::Deserializer::parseCDataNode()]: '<![CDATA[' expected");
    return;
  }

  auto label = state.caret->putLabel();

  if(!state.caret->findText("]]>", 3)) {
    state.errorStack.push("[oatpp::xml::Deserializer::parseCDataNode()]: unterminated CDATA node");
    return;
  }

  state.tree->setString(label.toString());
  state.caret->inc(3);
  name = "!CDATA";
}

void Deserializer::parseElementContent(State& state, const oatpp::String& name) {

  std::vector<std::pair<oatpp::String, data::mapping::Tree>> nodes;

  auto data = state.caret->getData();
  auto size = state.caret->getDataSize();

  bool hasText = false;
  auto label = state.caret->putLabel();

  v_buff_size i = state.caret->getPosition();
  while( i < size) {

    auto c = data[i];

    if(!hasText) {
      bool isWhitespace = (c == ' ' || c == '\r' || c == '\n' || c == '\t' || c == '\f');
      if (!isWhitespace && c != '<') {
        hasText = true;
      }
    }

    if(c == '<') {

      state.caret->setPosition(i);

      if(hasText) {
        data::mapping::Tree node;
        node.setString(label.toString());
        nodes.emplace_back("!TEXT", std::move(node));
      }

      if(state.caret->isAtText("</", 2, true)) {
        if(!state.caret->isAtText(name->c_str(), static_cast<v_buff_size>(name->size()), true)) {
          state.errorStack.push("[oatpp::xml::Deserializer::parseElementContent()]: Invalid closing for tag '" + name + "'");
          return;
        }
        state.caret->skipBlankChars();
        if(!state.caret->canContinueAtChar('>', 1)) {
          state.errorStack.push("[oatpp::xml::Deserializer::parseElementContent()]: Invalid closing for tag '" + name + "' - '>' expected.");
          return;
        }
        break;
      }

      nodes.emplace_back();

      State nestedState;
      nestedState.caret = state.caret;
      nestedState.config = state.config;
      nestedState.tree = &nodes[nodes.size() - 1].second;

      oatpp::String nestedName;
      parseNode(nestedState, nestedName);

      if(!nestedState.errorStack.empty()) {
        state.errorStack.splice(nestedState.errorStack);
        state.errorStack.push("[oatpp::xml::Deserializer::parseElementContent()]");
        return;
      }

      nodes[nodes.size() - 1].first = nestedName;

      i = state.caret->getPosition();
      label = state.caret->putLabel();

    } else {
      i ++;
    }

  }

  if(!nodes.empty()) {
    if(nodes.size() == 1 && nodes[0].first == "!TEXT") {
      state.tree->setString(nodes[0].second.getString());
    } else {
      state.tree->setPairs(nodes);
    }
  }

}

void Deserializer::parseElementNode(State& state, oatpp::String& name) {

  if(!state.caret->isAtText("<", 1, true)) {
    state.errorStack.push("[oatpp::xml::Deserializer::parseElementNode()]: '<' expected");
    return;
  }

  name = parseElementName(state);
  if(!state.errorStack.empty()) {
    state.errorStack.push("[oatpp::xml::Deserializer::parseElementNode()]");
    return;
  }

  parseAttributes(state);
  if(!state.errorStack.empty()) {
    state.errorStack.push("[oatpp::xml::Deserializer::parseElementNode()]: tag='" + name + "'");
    return;
  }

  if(state.caret->isAtChar('/')) {
    if(!(state.caret->canContinueAtChar('/', 1) && state.caret->canContinueAtChar('>', 1))) {
      state.errorStack.push("[oatpp::xml::Deserializer::parseElementNode()]: tag='" + name + "' - '/>' expected");
    }
    return;
  }

  if(!state.caret->canContinueAtChar('>', 1)) {
    state.errorStack.push("[oatpp::xml::Deserializer::parseElementNode()]: tag='" + name + "' - '>' expected");
    return;
  }

  parseElementContent(state, name);
  if(!state.errorStack.empty()) {
    state.errorStack.push("[oatpp::xml::Deserializer::parseElementNode()]: tag='" + name + "'");
    return;
  }

}

void Deserializer::parseNode(State& state, oatpp::String& name) {

  if(state.caret->isAtText("<?", 2, false)) {
    parsePINode(state, name);
  } else if(state.caret->isAtText("<!--", 4, false)) {
    parseCommentNode(state, name);
  } else if(state.caret->isAtText("<![CDATA[", 9, false)) {
    parseCDataNode(state, name);
  } else if( state.caret->isAtChar('<')) {
    parseElementNode(state, name);
  } else {
    state.errorStack.push("[oatpp::xml::Deserializer::parseNode()]: '<' expected");
    return;
  }

}

void Deserializer::deserialize(oatpp::xml::Deserializer::State &state) {

  state.tree->setPairs({});
  auto& pairs = state.tree->getPairs();

  state.caret->skipBlankChars();

  while (state.caret->canContinue()) {

    data::mapping::Tree nestedNode;

    State nestedState;
    nestedState.caret = state.caret;
    nestedState.config = state.config;
    nestedState.tree = &nestedNode;

    oatpp::String nestedName;
    parseNode(nestedState, nestedName);

    if(!nestedState.errorStack.empty()) {
      state.errorStack.splice(nestedState.errorStack);
      state.errorStack.push("[oatpp::xml::Deserializer::deserialize()]");
      return;
    }

    pairs.emplace_back(nestedName, std::move(nestedNode));

    state.caret->skipBlankChars();

  }

}


}}
