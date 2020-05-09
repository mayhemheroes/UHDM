/*
 Do not modify, auto-generated by model_gen.tcl

 Copyright 2019-2020 Alain Dargelas

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

/*
 * File:   vpi_visitor.cpp
 * Author:
 *
 * Created on December 14, 2019, 10:03 PM
 */

#include <string.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <set>

#include "include/sv_vpi_user.h"
#include "include/vhpi_user.h"

#include "headers/uhdm_types.h"
#include "headers/containers.h"
#include "headers/vpi_uhdm.h"
#include "headers/uhdm.h"
#include "headers/Serializer.h"

namespace UHDM {

std::string visit_value(s_vpi_value* value) {
  if (value == nullptr)
    return "";
  switch (value->format) {
  case vpiIntVal: {
    return std::string(std::string("|INT:") + std::to_string(value->value.integer) + "\n");
    break;
  }
  case vpiStringVal: {
    const char* s = (const char*) value->value.str;
    return std::string(std::string("|STRING:") + std::string(s) + "\n");
    break;
  }
  case vpiBinStrVal: {
    const char* s = (const char*) value->value.str;
    return std::string(std::string("|BIN:") + std::string(s) + "\n");
    break;
  }
  case vpiHexStrVal: {
    const char* s = (const char*) value->value.str;
    return std::string(std::string("|HEX:") + std::string(s) + "\n");
    break;
  }
  case vpiOctStrVal: {
    const char* s = (const char*) value->value.str;
    return std::string(std::string("|OCT:") + std::string(s) + "\n");
    break;
  }
  case vpiRealVal: {
    return std::string(std::string("|REAL:") + std::to_string(value->value.real) + "\n");
    break;
  }
  case vpiScalarVal: {
    return std::string(std::string("|SCAL:") + std::to_string(value->value.scalar) + "\n");
    break;
  } 
  default:
    break;
  }
  return "";
}

std::string visit_delays(s_vpi_delay* delay) {
  if (delay == nullptr)
    return "";
  switch (delay->time_type) {
  case vpiScaledRealTime: {
    return std::string(std::string("|#") + std::to_string(delay->da[0].low) + "\n");
    break;
  }
  default:
    break;
  }
  return "";
}  

std::string visit_object (vpiHandle obj_h, unsigned int indent, const std::string& relation, std::set<const BaseClass*>& visited) {
  std::string result;
  unsigned int subobject_indent = indent + 2;
  std::string hspaces;
  std::string rspaces;
  const uhdm_handle* const handle = (const uhdm_handle*) obj_h;
  const BaseClass* const object = (const BaseClass*) handle->object;
  bool alreadyVisited = false;
  if (visited.find(object) != visited.end()) {
    alreadyVisited = true;
  }
  visited.insert(object);
  if (indent > 0) {
    for (unsigned int i = 0; i < indent -2 ; i++) {
      hspaces += " ";
    }
    rspaces = hspaces + "|";
    hspaces += "\\_";
  }
  std::string spaces;
  for (unsigned int i = 0; i < indent; i++)
    spaces += " ";
  std::string objectName = ""; // Instance name
  std::string defName    = ""; // Definition name
  std::string fileName = "";
  std::string lineNo   = "";
  std::string parent   = "";
  std::string uhdmid   = ", id:" + std::to_string(object->UhdmId());
  if (unsigned int l = vpi_get(vpiLineNo, obj_h)) {
    lineNo = ", line:" + std::to_string(l);
  }
  const unsigned int objectType = vpi_get(vpiType, obj_h);				     
  if (objectType == vpiModule || objectType == vpiProgram || objectType == vpiClassDefn || objectType == vpiPackage ||
      objectType == vpiInterface || objectType == vpiUdp) {
    if (const char* s = vpi_get_str(vpiFile, obj_h))
      fileName = ", file:" +  std::string(s);
  }
  if (vpiHandle par = vpi_handle(vpiParent, obj_h)) {
    if (const char* parentName = vpi_get_str(vpiName, par)) {
      parent = ", parent:" + std::string(parentName);
    }
    vpi_free_object(par);
  }
  if (const char* s = vpi_get_str(vpiDefName, obj_h)) {
    defName = s;
  }
  if (const char* s = vpi_get_str(vpiName, obj_h)) {
    if (defName != "") {
      defName += " ";
    }
    objectName = std::string("(") + s + std::string(")");
  }
  if (relation != "") {
    result += rspaces + relation + ":\n";
  }
  result += hspaces + UHDM::VpiTypeName(obj_h) + ": " + defName + objectName + uhdmid + fileName + lineNo + parent + "\n";
  if (alreadyVisited) {
    return result;
  }
  if (relation == "vpiParent") {
    return result;
  }
<OBJECT_VISITORS>
  return result;
}

std::string visit_designs (const std::vector<vpiHandle>& designs) {
  std::string result;
  for (auto design : designs) {
    std::set<const BaseClass*> visited;
    result += visit_object(design, 0, "", visited);
  }
  return result;
}

};
