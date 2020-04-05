#include "fake-jni/jvm.h"

namespace FakeJni {
 const JClass & JObject::getClass() const noexcept {
  return *descriptor;
 }

 std::shared_ptr<const JClass> JObject::getClassRef() const noexcept {
  return descriptor;
 }
}

DEFINE_NATIVE_DESCRIPTOR(FakeJni::JObject)END_NATIVE_DESCRIPTOR