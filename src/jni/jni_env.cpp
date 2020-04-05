#include "fake-jni/jvm.h"
#include <fake-jni/jvm.h>

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
  localFrames.emplace_back(frameSize);
 }

 void JniEnv::popLocalFrame() {
  localFrames.pop_back();
 }

 void JniEnv::ensureLocalCapacity(size_t frameSize) {
  localFrames.back().ensureSize(frameSize);
 }

 jobject JniEnv::createLocalReference(std::shared_ptr<JObject> object) {
  size_t index = localFrames.back().createReference(std::move(object));
  return JniReferenceDescription(index, false).ptr;
 }

 void JniEnv::deleteLocalReference(jobject reference) {
  auto desc = JniReferenceDescription(reference).desc;
  if (desc.isGlobal)
   throw std::runtime_error("FATAL: Reference is a global reference");
  auto it = std::lower_bound(localFrames.begin(), localFrames.end(), desc.index,
   [](auto const &frame, size_t index) {
    return frame.getStart() < index;
   });
  if (it == localFrames.end())
   throw std::runtime_error("FATAL: Reference index has no matching frame");
  it->deleteReference(desc.index);
 }

 std::shared_ptr<JObject> JniEnv::resolveReference(jobject reference) {
  auto desc = JniReferenceDescription(reference).desc;
  if (desc.isGlobal)
   return vm.getGlobalReferenceTable().getReference(desc.index);
  auto it = std::lower_bound(localFrames.begin(), localFrames.end(), desc.index,
   [](auto const &frame, size_t index) {
    return frame.getStart() < index;
   });
  if (it == localFrames.end())
   return std::shared_ptr<JObject>();
  return it->getReference(desc.index);
 }
}