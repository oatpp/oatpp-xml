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

#include "Serializer.hpp"
#include "oatpp/utils/Conversion.hpp"

namespace oatpp { namespace xml {

void Serializer::startNode(const oatpp::String& name, State& state) {
  state.stream->writeSimple("<", 1);
  state.stream->writeSimple(name);
  for(v_uint32 i = 0; i < state.tree->attributes().size(); i ++) {
    auto attr = state.tree->attributes()[i];
    state.stream->writeSimple(" ", 1);
    state.stream->writeSimple(attr.first);
    state.stream->writeSimple("=\"", 2);
    state.stream->writeSimple(Utils::escapeAttributeText(attr.second.get(), '"', state.errorStack));
    state.stream->writeSimple("\"", 1);
  }
  state.stream->writeSimple(">", 1);
}

void Serializer::endNode(const oatpp::String& name, State& state) {
  state.stream->writeSimple("</", 2);
  state.stream->writeSimple(name);
  state.stream->writeSimple(">", 1);
}

void Serializer::serializeCData(State& state) {

  auto& node = *state.tree;

  if(node.isNull()) {
    return;
  }

  if(node.getType() != data::mapping::Tree::Type::STRING) {
    state.errorStack.push("[oatpp::xml::Serializer::serializeCData()]: String is expected as CDATA data");
    return;
  }

  state.stream->writeSimple("<![CDATA[", 9);
  auto data = state.tree->getString();
  state.stream->writeSimple(data->data(), static_cast<v_buff_size>(data->size()));
  state.stream->writeSimple("]]>", 3);

}

void Serializer::serializeComment(State& state) {

  auto& node = *state.tree;

  if(node.isNull()) {
    return;
  }

  if(node.getType() != data::mapping::Tree::Type::STRING) {
    state.errorStack.push("[oatpp::xml::Serializer::serializeComment()]: String is expected as COMMENT data");
    return;
  }

  state.stream->writeSimple("<!--", 4);
  auto data = state.tree->getString();
  state.stream->writeSimple(data->data(), static_cast<v_buff_size>(data->size()));
  state.stream->writeSimple("-->", 3);

}

void Serializer::serializePINode(State& state, const oatpp::String& key) {

  if(!key || key->size() < 2) {
    state.errorStack.push("[oatpp::xml::Serializer::serializePINode()]: Invalid tag name");
    return;
  }

  auto& node = *state.tree;

  if(node.getType() != data::mapping::Tree::Type::STRING) {
    state.errorStack.push("[oatpp::xml::Serializer::serializePINode()]: String is expected as PI data");
    return;
  }

  state.stream->writeSimple("<?", 2);
  state.stream->writeSimple(key->data() + 1, static_cast<v_buff_size>(key->size() - 1));

  auto data = state.tree->getString();
  if(!data->empty()) {
    state.stream->writeSimple(" ", 1);
    state.stream->writeSimple(data->data(), static_cast<v_buff_size>(data->size()));
  }

  state.stream->writeSimple("?>", 2);

}

bool Serializer::serializeSpecial(State& state, const oatpp::String& key) {
  if(key->empty()) return false;

  auto c = key->data()[0];

  switch (c) {
    case '!': {
      if(key == "!TEXT") {
        serializeString(state);
        return true;
      }
      if(key == "!CDATA") {
        serializeCData(state);
        return true;
      }
      if(key == "!COMMENT") {
        serializeComment(state);
        return true;
      }
      state.errorStack.push("[oatpp::xml::Serializer::serializeSpecial()]: Unknown special node type '" + key + "'");
      break;
    }
    case '?': {
      serializePINode(state, key);
      return true;
    }
    default:
      break;
  }

  return false;
}

void Serializer::serializeString(State& state) {
  auto content = Utils::escapeElementText(state.tree->getString(), state.errorStack);
  if(!state.errorStack.empty()) {
    state.errorStack.push("[oatpp::xml::Serializer::serializeString()]: Can't escape string");
    return;
  }
  state.stream->writeSimple(content);
}

void Serializer::serializeArray(State& state) {

  State nestedState;
  nestedState.stream = state.stream;
  nestedState.config = state.config;

  auto& vector = state.tree->getVector();

  oatpp::String itemName = "item";

  v_int64 index = 0;
  for(auto& tree : vector) {

    nestedState.tree = &tree;

    if(!tree.isNull() || state.config->includeNullElements) {

      startNode(itemName, nestedState);
      if(!nestedState.errorStack.empty()) {
        state.errorStack.splice(nestedState.errorStack);
        state.errorStack.push("[oatpp::xml::Serializer::serializeArray()]: index=" + utils::Conversion::int64ToStr(index));
        return;
      }

      serialize(nestedState);

      if(!nestedState.errorStack.empty()) {
        state.errorStack.splice(nestedState.errorStack);
        state.errorStack.push("[oatpp::xml::Serializer::serializeArray()]: index=" + utils::Conversion::int64ToStr(index));
        return;
      }

      endNode(itemName, state);

    }

    index ++;

  }

}

void Serializer::serializeMap(State& state) {

  State nestedState;
  nestedState.stream = state.stream;
  nestedState.config = state.config;

  auto& map = state.tree->getMap();
  auto mapSize = map.size();

  for(v_uint64 index = 0; index < mapSize; index ++) {

    const auto& pair = map[index];

    nestedState.tree = &pair.second.get();

    if(!nestedState.tree->isNull() || state.config->includeNullElements) {

      startNode(pair.first, nestedState);
      if(!nestedState.errorStack.empty()) {
        state.errorStack.splice(nestedState.errorStack);
        state.errorStack.push("[oatpp::xml::Serializer::serializeMap()]: key='" + pair.first + "'");
        return;
      }

      serialize(nestedState);

      if(!nestedState.errorStack.empty()) {
        state.errorStack.splice(nestedState.errorStack);
        state.errorStack.push("[oatpp::xml::Serializer::serializeMap()]: key='" + pair.first + "'");
        return;
      }

      endNode(pair.first, nestedState);

    }

  }

}

void Serializer::serializePairs(State& state) {

  State nestedState;
  nestedState.stream = state.stream;
  nestedState.config = state.config;

  auto& map = state.tree->getPairs();
  auto mapSize = map.size();

  for(v_uint64 index = 0; index < mapSize; index ++) {

    const auto& pair = map[index];

    nestedState.tree = &pair.second;

    if(!nestedState.tree->isNull() || state.config->includeNullElements) {

      auto& key = pair.first;
      if(serializeSpecial(nestedState, key)) {
        if(!nestedState.errorStack.empty()) {
          state.errorStack.splice(nestedState.errorStack);
          state.errorStack.push("[oatpp::xml::Serializer::serializePairs()]: key='" + key + "'");
          return;
        }
        continue;
      }

      startNode(key, nestedState);
      if(!nestedState.errorStack.empty()) {
        state.errorStack.splice(nestedState.errorStack);
        state.errorStack.push("[oatpp::xml::Serializer::serializePairs()]: key='" + key + "'");
        return;
      }

      serialize(nestedState);

      if(!nestedState.errorStack.empty()) {
        state.errorStack.splice(nestedState.errorStack);
        state.errorStack.push("[oatpp::xml::Serializer::serializePairs()]: key='" + key + "'");
        return;
      }

      endNode(key, nestedState);

    }

  }

}

void Serializer::serialize(oatpp::xml::Serializer::State &state) {

  switch (state.tree->getType()) {

    case data::mapping::Tree::Type::UNDEFINED:
//      state.errorStack.push("[oatpp::xml::Serializer::serialize()]: "
//                            "UNDEFINED tree node is NOT serializable. To fix: set node value.");
      return;
    case data::mapping::Tree::Type::NULL_VALUE: state.stream->writeSimple("null", 4); return;

    case data::mapping::Tree::Type::INTEGER: state.stream->writeAsString(state.tree->getInteger()); return;
    case data::mapping::Tree::Type::FLOAT: state.stream->writeAsString(state.tree->getFloat()); return;

    case data::mapping::Tree::Type::BOOL:  state.stream->writeAsString(state.tree->getPrimitive<bool>()); return;

    case data::mapping::Tree::Type::INT_8: state.stream->writeAsString(state.tree->getPrimitive<v_int8>()); return;
    case data::mapping::Tree::Type::UINT_8: state.stream->writeAsString(state.tree->getPrimitive<v_uint8>()); return;
    case data::mapping::Tree::Type::INT_16: state.stream->writeAsString(state.tree->getPrimitive<v_int16>()); return;
    case data::mapping::Tree::Type::UINT_16: state.stream->writeAsString(state.tree->getPrimitive<v_uint16>()); return;
    case data::mapping::Tree::Type::INT_32: state.stream->writeAsString(state.tree->getPrimitive<v_int32>()); return;
    case data::mapping::Tree::Type::UINT_32: state.stream->writeAsString(state.tree->getPrimitive<v_uint32>()); return;
    case data::mapping::Tree::Type::INT_64: state.stream->writeAsString(state.tree->getPrimitive<v_int64>()); return;
    case data::mapping::Tree::Type::UINT_64: state.stream->writeAsString(state.tree->getPrimitive<v_uint64>()); return;

    case data::mapping::Tree::Type::FLOAT_32: state.stream->writeAsString(state.tree->getPrimitive<v_float32>()); return;
    case data::mapping::Tree::Type::FLOAT_64: state.stream->writeAsString(state.tree->getPrimitive<v_float64>()); return;

    case data::mapping::Tree::Type::STRING: serializeString(state); return;
    case data::mapping::Tree::Type::VECTOR: serializeArray(state); return;
    case data::mapping::Tree::Type::MAP: serializeMap(state); return;
    case data::mapping::Tree::Type::PAIRS: serializePairs(state); return;

    default:
      break;

  }

  state.errorStack.push("[oatpp::xml::Serializer::serialize()]: Unknown node type");

}

}}
