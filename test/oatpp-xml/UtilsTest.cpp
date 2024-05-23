/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *                         Benedikt-Alexander Mokroß <oatpp@bamkrs.de>
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

#include "UtilsTest.hpp"

#include "oatpp-xml/ObjectMapper.hpp"
#include "oatpp-xml/Utils.hpp"

#include "oatpp/macro/codegen.hpp"

namespace oatpp { namespace xml {

#include OATPP_CODEGEN_BEGIN(DTO)

class TestObj1 : public oatpp::DTO {

  DTO_INIT(TestObj1, DTO)

  DTO_FIELD(String, text);
  DTO_FIELD(Int32, intValue);

};

class TestObj2 : public oatpp::DTO {

  DTO_INIT(TestObj2, DTO)

  DTO_FIELD(String, textValue);

  DTO_FIELD(Object<TestObj1>, object);
  DTO_FIELD(List<Object<TestObj1>>, list);
  DTO_FIELD(Fields<Object<TestObj1>>, map);

};

#include OATPP_CODEGEN_END(DTO)

void UtilsTest::onRun() {

  {
    data::mapping::ErrorStack errorStack;
    auto res = Utils::escapeAttributeText("\"red heart\" emoji ❤\uFE0F is composed 😍 + 💚 + ❤️", '"', errorStack);
    if (!errorStack.empty()) {
      OATPP_LOGe(TAG, "errorStack: \n{}", errorStack.stacktrace())
    }
    OATPP_LOGd(TAG, "res='{}'", res)
    OATPP_LOGd(TAG, "ures='{}'", Utils::unescapeText(res, errorStack));
  }

  {
    data::mapping::ErrorStack errorStack;
    auto res = Utils::escapeElementText("\"red heart\" emoji ❤\uFE0F is composed\n\n 😍 + 💚 + ❤️a", errorStack);
    if (!errorStack.empty()) {
      OATPP_LOGe(TAG, "errorStack: \n{}", errorStack.stacktrace())
    }
    OATPP_LOGd(TAG, "res='{}'", res)
    OATPP_LOGd(TAG, "ures='{}'", Utils::unescapeText(res, errorStack));
  }

  {
    data::mapping::ErrorStack errorStack;
    OATPP_LOGd(TAG, "ures='{}'", Utils::unescapeText("Line1&#10;Line2", errorStack));
  }

  {
    ObjectMapper mapper;
    oatpp::Tree tree;
    auto& node = *tree;
    node["name"] = "Oat++";
    node["age"] = 3;

    node["name"].attributes()["local"] = "en";
    node["name"].attributes()["charset"] = "utf-8";

    node["children"].setVector(3);
    node["children"][0] = 0;
    node["children"][1] = 1;
    node["children"][2] = 2;

    //auto obj = TestObj2::createShared();

    auto xml = mapper.writeToString(oatpp::Fields<oatpp::Any>({{"root", tree}}));
    OATPP_LOGd(TAG, "xml:'\n{}'", xml)
  }

}

}}
