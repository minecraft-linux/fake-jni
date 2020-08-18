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
  native = std::unique_ptr<NativeInterface>(ni);
  functions = ni;
 }

 inline NativeInterface& JniEnv::getNativeInterface() const {
  return *native;
 }

 void JniEnv::pushLocalFrame(size_t frameSize) {
  auto start = 1;
  if (!localFrames.empty())
   start = localFrames.back().getStart() + localFrames.back().getSize();
  localFrames.emplace_back(JniReferenceTable(frameSize, start));
 }

 void JniEnv::popLocalFrame() {
  localFrames.pop_back();
 }

 void JniEnv::ensureLocalCapacity(size_t frameSize) {
  localFrames.back().ensureSize(frameSize);
 }

 jobject JniEnv::createLocalReference(std::shared_ptr<JObject> object) {
  if (!object)
      return nullptr;
  size_t index = localFrames.back().createReference(std::move(object));
  return JniReferenceDescription(index, false).ptr;
 }

 void JniEnv::deleteLocalReference(jobject reference) {
  auto desc = JniReferenceDescription(reference).desc;
  if (desc.isGlobal)
   throw std::runtime_error("FATAL: Reference is a global reference");
  auto it = std::upper_bound(localFrames.begin(), localFrames.end(), desc.index,
   [](size_t index, auto const &frame) {
    return index < frame.getStart();
   });
  if (it == localFrames.begin())
   throw std::runtime_error("FATAL: Reference index has no matching frame");
  --it;
  it->deleteReference(desc.index);
 }

 std::shared_ptr<JObject> JniEnv::resolveReference(jobject reference) const {
  auto desc = JniReferenceDescription(reference).desc;
  if (desc.isGlobal)
   return vm.getGlobalReferenceTable().getReference(desc.index);
  auto it = std::upper_bound(localFrames.begin(), localFrames.end(), desc.index,
   [](size_t index, auto const &frame) {
    return index < frame.getStart();
   });
  if (it == localFrames.begin())
   return std::shared_ptr<JObject>();
  --it;
  return it->getReference(desc.index);
 }
}