#include "fake-jni/jvm.h"

#include <stdexcept>

namespace FakeJni {
 jint NativeInterface::ensureLocalCapacity(jint capacity) const {
     env.ensureLocalCapacity(capacity);
     return 0;
 }

 jint NativeInterface::getVersion() const {
  return JNI_VERSION_1_8;
 }

 jint NativeInterface::registerNatives(jclass jclazz, const JNINativeMethod * methods, const jint numMethods) const {
  bool success = true;
  const auto clazz = std::dynamic_pointer_cast<JClass>(env.resolveReference(jclazz));
  for (int i = 0; i < numMethods; i++) {
   const auto method = &methods[i];
#ifdef FAKE_JNI_DEBUG
   printf(
    "DEBUG: JVMNativeInterface_::registerNatives: Registering native method:\n\t0x%lx [%s, %s, 0x%lx]\n",
    (uintptr_t)method,
    method->name,
    method->signature,
    (uintptr_t)method->fnPtr
   );
#endif
   success &= clazz->registerMethod(new JMethodID(method));
  }
  return success ? JNI_OK : -1;
 }

 jint NativeInterface::unregisterNatives(jclass const jclazz) const {
  bool success = true;
  const auto clazz = std::dynamic_pointer_cast<JClass>(env.resolveReference(jclazz));
  std::vector<const JMethodID *> toRemove;
  for (auto& mid : clazz->getMethods()) {
   if (mid->type == JMethodID::REGISTER_NATIVES_FUNC) {
    toRemove.push_back(mid);
   }
  }
  for (auto const mid : toRemove) {
   success &= clazz->unregisterMethod(mid);
  }
  return success ? 0 : -1;
 }

//TODO implement
 jint NativeInterface::monitorEnter(jobject) const {
  throw std::runtime_error("FATAL: 'JVMNativeInterface_::monitorEnter' is unimplemented!");
  return 0;
 }

//TODO implement
 jint NativeInterface::monitorExit(jobject) const {
  throw std::runtime_error("FATAL: 'JVMNativeInterface_::monitorExit' is unimplemented!");
  return 0;
 }

 jint NativeInterface::getJavaVM(JavaVM **pVm) const {
  *pVm = &vm;
  return 0;
 }
}