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
  localFrames.push(JniLocalFrame(frameSize));
 }

 void JniEnv::popLocalFrame() {
  localFrames.pop();
 }

 void JniEnv::ensureLocalCapacity(size_t frameSize) {
  localFrames.top().ensureSize(frameSize);
 }

 JniEnv::JniLocalFrame::JniLocalFrame(size_t size) {
  resizeFrame(size);
 }

 void JniEnv::JniLocalFrame::resizeFrame(size_t size) {
  if (size < references.size())
   throw std::runtime_error("FATAL: Growing local frame down is not supported");
  auto oldSize = references.size();
  references.resize(size);
  referencesNextIndex.reserve(size + 1);
  if (referencesNextIndex.empty())
   referencesNextIndex.push_back(1);
  for (auto i = 1 + oldSize; i < size; i++)
   referencesNextIndex.push_back(i + 1);
  if (size > oldSize)
   referencesNextIndex.push_back(0);
  referencesNextIndex[lastReferenceIndex] = 1 + oldSize;
  lastReferenceIndex = size;
 }

 void JniEnv::JniLocalFrame::ensureSize(size_t size) {
  if (size > references.size())
   resizeFrame(size);
 }

 size_t JniEnv::JniLocalFrame::reserveReference() {
  auto ret = referencesNextIndex[0];
  referencesNextIndex[0] = referencesNextIndex[ret];
  referencesNextIndex[ret] = 0;
  return ret;
 }

 void JniEnv::JniLocalFrame::returnReference(size_t index) {
  referencesNextIndex[lastReferenceIndex] = index;
  lastReferenceIndex = index;
 }
}