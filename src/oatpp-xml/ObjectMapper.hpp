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


#ifndef OATPP_XML_OBJECTMAPPER_HPP
#define OATPP_XML_OBJECTMAPPER_HPP

#include "./Serializer.hpp"
#include "./Deserializer.hpp"

#include "oatpp/data/mapping/ObjectToTreeMapper.hpp"
#include "oatpp/data/mapping/TreeToObjectMapper.hpp"
#include "oatpp/data/mapping/ObjectMapper.hpp"

namespace oatpp { namespace xml {

class ObjectMapper : public oatpp::base::Countable, public oatpp::data::mapping::ObjectMapper {
private:
  static Info getMapperInfo() {
    return Info("application", "xml");
  }

public:

  class DeserializerConfig {
  public:
    data::mapping::TreeToObjectMapper::Config mapper;
    Deserializer::Config xml;
  };

public:

  class SerializerConfig {
  public:
    data::mapping::ObjectToTreeMapper::Config mapper;
    Serializer::Config xml;
  };

private:
  void writeTree(data::stream::ConsistentOutputStream* stream, const data::mapping::Tree& tree, data::mapping::ErrorStack& errorStack) const;
private:
  SerializerConfig m_serializerConfig;
  DeserializerConfig m_deserializerConfig;
private:
  data::mapping::ObjectToTreeMapper m_objectToTreeMapper;
  data::mapping::TreeToObjectMapper m_treeToObjectMapper;
public:

  ObjectMapper(const SerializerConfig& serializerConfig = {}, const DeserializerConfig& deserializerConfig = {});

  void write(data::stream::ConsistentOutputStream* stream, const oatpp::Void& variant, data::mapping::ErrorStack& errorStack) const override;

  oatpp::Void read(oatpp::utils::parser::Caret& caret, const oatpp::Type* type, data::mapping::ErrorStack& errorStack) const override;

  const SerializerConfig& serializerConfig() const;
  const DeserializerConfig& deserializerConfig() const;

  SerializerConfig& serializerConfig();
  DeserializerConfig& deserializerConfig();

};

}}

#endif /* OATPP_XML_OBJECTMAPPER_HPP */
