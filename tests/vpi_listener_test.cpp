// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-

#include <iostream>
#include <memory>
#include <stack>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

// uhdm
#include "uhdm/VpiListener.h"
#include "uhdm/VpiListenerTracer.h"

// We include this last to make sure that the headers above don't accidentally
// depend on any class defined here
#include "uhdm/uhdm.h"

using namespace UHDM;
using testing::ElementsAre;
using testing::HasSubstr;

class MyVpiListener : public VpiListener {
 protected:
  void enterModule(const module* object, vpiHandle handle) override {
    CollectLine("Module", object);
    stack_.push(object);
  }

  void leaveModule(const module* object, vpiHandle handle) override {
    ASSERT_EQ(stack_.top(), object);
    stack_.pop();
  }

  void enterProgram(const program* object, vpiHandle handle) override {
    CollectLine("Program", object);
    stack_.push(object);
  }

  void leaveProgram(const program* object, vpiHandle handle) override {
    ASSERT_EQ(stack_.top(), object);
    stack_.pop();
  }

 public:
  void CollectLine(const std::string& prefix, const BaseClass* object) {
    vpiHandle parentHandle = NewVpiHandle(object->VpiParent());
    const char* const parentName = vpi_get_str(vpiName, parentHandle);
    vpi_free_object(parentHandle);
    collected_.push_back(
        prefix + ": " + object->VpiName() + "/" + object->VpiDefName() +
        " parent: " + ((parentName != nullptr) ? parentName : "-"));
  }

  const std::vector<std::string>& collected() const { return collected_; }

 private:
  std::vector<std::string> collected_;
  std::stack<const BaseClass*> stack_;
};

static std::vector<vpiHandle> buildModuleProg(Serializer* s) {
  // Design building
  design* d = s->MakeDesign();
  d->VpiName("design1");
  // Module
  module* m1 = s->MakeModule();
  m1->VpiTopModule(true);
  m1->VpiDefName("M1");
  m1->VpiFullName("top::M1");
  m1->VpiParent(d);

  // Module
  module* m2 = s->MakeModule();
  m2->VpiDefName("M2");
  m2->VpiName("u1");
  m2->VpiParent(m1);

  // Module
  module* m3 = s->MakeModule();
  m3->VpiDefName("M3");
  m3->VpiName("u2");
  m3->VpiParent(m1);

  // Instance
  module* m4 = s->MakeModule();
  m4->VpiDefName("M4");
  m4->VpiName("u3");
  m4->VpiParent(m3);
  m4->Instance(m3);

  VectorOfmodule* v1 = s->MakeModuleVec();
  v1->push_back(m1);
  d->AllModules(v1);

  VectorOfmodule* v2 = s->MakeModuleVec();
  v2->push_back(m2);
  v2->push_back(m3);
  m1->Modules(v2);

  // Package
  package* p1 = s->MakePackage();
  p1->VpiName("P1");
  p1->VpiDefName("P0");
  VectorOfpackage* v3 = s->MakePackageVec();
  v3->push_back(p1);
  d->AllPackages(v3);

  // Instance items, illustrates the use of groups
  program* pr1 = s->MakeProgram();
  pr1->VpiDefName("PR1");
  pr1->VpiParent(m1);
  VectorOfany* inst_items = s->MakeAnyVec();
  inst_items->push_back(pr1);
  m1->Instance_items(inst_items);

  return {s->MakeUhdmHandle(uhdmdesign, d)};
}

TEST(VpiListenerTest, ProgramModule) {
  Serializer serializer;
  const std::vector<vpiHandle>& design = buildModuleProg(&serializer);

  std::unique_ptr<MyVpiListener> listener(new MyVpiListener());
  listener->listenDesigns(design);
  const std::vector<std::string> expected = {
      "Module: /M1 parent: design1",
      "Program: /PR1 parent: -",
      "Module: u1/M2 parent: -",
      "Module: u2/M3 parent: -",
  };
  EXPECT_EQ(listener->collected(), expected);
}

TEST(UhdmListenerTracerTest, ProgramModule) {
  Serializer serializer;
  const std::vector<vpiHandle>& design = buildModuleProg(&serializer);

  std::stringstream out;
  std::unique_ptr<VpiListenerTracer> listener(new VpiListenerTracer(out));
  listener->listenDesigns(design);
  EXPECT_THAT(out.str(), HasSubstr("enterDesign: [0,0:0,0]"));
}
