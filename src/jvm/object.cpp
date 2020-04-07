#include "fake-jni/jvm.h"

namespace FakeJni {
 const JClass & JObject::getClass() const noexcept {
  return *getDescriptor();
 }

 std::shared_ptr<const JClass> JObject::getClassRef() const noexcept {
  return getDescriptor();
 }

 const std::shared_ptr<const JClass>& JObject::getDescriptor() noexcept {
  static const std::shared_ptr<const JClass> descriptor = []() {
   std::shared_ptr<JClass> ptr (new FakeJni::JClass { JClass::PUBLIC, _CX::JClassBreeder<JObject> {

   }});
   ptr->parent = ptr;
   return ptr;
  }();
  return descriptor;
 }
}
