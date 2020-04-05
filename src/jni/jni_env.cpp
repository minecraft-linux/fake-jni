#include "fake-jni/jvm.h"

namespace FakeJni {
 thread_local JniEnv * JniEnv::currentEnv;

 JniEnv* JniEnv::getCurrentEnv() noexcept {
  return currentEnv;
 }

 JniEnv::JniEnv(const Jvm& vm) noexcept :
  JNIEnv(),
  vm(const_cast<Jvm &>(vm))
 {
  setNativeInterface<NativeInterface>();
  currentEnv = this;
 }

 JniEnv::~JniEnv() {
  if (currentEnv == this)
    currentEnv = nullptr;
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

 void JniEnv::pushLocalFrame(size_t frameSize) {
  localFrames.push(JniReferenceTable(frameSize));
 }

 void JniEnv::popLocalFrame() {
  localFrames.pop();
 }

 void JniEnv::ensureLocalCapacity(size_t frameSize) {
  localFrames.top().ensureSize(frameSize);
 }
}