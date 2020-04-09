#include "fake-jni/jvm.h"
#include "fake-jni/weak.h"

#include <algorithm>
#include <fake-jni/jvm.h>
#include <stdexcept>

namespace FakeJni {
 jobjectRefType NativeInterface::getObjectRefType(jobject const jobj) const {
  if (!env.resolveReference(jobj))
   return jobjectRefType::JNIInvalidRefType;
  if (JniReferenceDescription(jobj).desc.isGlobal)
   return jobjectRefType::JNIGlobalRefType;
  return jobjectRefType::JNILocalRefType;
 }

 jobject NativeInterface::newGlobalRef(jobject const jobj) const {
  auto ref = env.resolveReference(jobj);
  if (!ref)
   throw std::runtime_error("FATAL: Invalid reference");
  return vm.createGlobalReference(ref);
 }

 void NativeInterface::deleteGlobalRef(jobject const jobj) const {
  if (jobj)
   vm.deleteGlobalReference(jobj);
 }

 jobject NativeInterface::newLocalRef(jobject const jobj) const {
  auto ref = env.resolveReference(jobj);
  if (!ref)
   throw std::runtime_error("FATAL: Invalid reference");
  return env.createLocalReference(ref);
 }

 void NativeInterface::deleteLocalRef(jobject const jobj) const {
  if (jobj)
   env.deleteLocalReference(jobj);
 }

 jweak NativeInterface::newWeakGlobalRef(jobject const jobj) const {
#ifdef FAKE_JNI_DEBUG
  fprintf(
   vm.getLog(),
   "WARNING: Creating a weak global reference, which is not supported"
  );
#endif
  return (jweak) newGlobalRef(jobj);
 }

 void NativeInterface::deleteWeakGlobalRef(jweak const weak) const {
#ifdef FAKE_JNI_DEBUG
  fprintf(
      vm.getLog(),
      "WARNING: Destroying a weak global reference, which is not supported"
  );
#endif
  deleteGlobalRef((jobject) weak);
 }
}