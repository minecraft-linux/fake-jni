#include "fake-jni/jvm.h"

namespace FakeJni {
 JniEnv::JniEnv(const Jvm& vm) noexcept :
  JNIEnv(),
  vm(const_cast<Jvm &>(vm))
 {
  setNativeInterface<NativeInterface>();
 }

 const Jvm& JniEnv::getVM() const noexcept {
  return vm;
 }

 inline void JniEnv::setNativeInterface(NativeInterface * const ni) {
  delete native;
  native = ni;
  functions = ni;
 }

 inline NativeInterface& JniEnv::getNativeInterface() const {
  return *native;
 }
}