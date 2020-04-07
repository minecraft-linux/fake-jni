#include "fake-jni/jvm.h"

namespace FakeJni {
 jint InvokeInterface::getEnv(Jvm * vm, void ** penv, jint version) const {
  auto env = JniEnv::getCurrentEnv();
  if (env == nullptr || &env->vm != vm) {
   *penv = nullptr;
   return JNI_EDETACHED;
  }
  *penv = (void *)((JNIEnv *)env);
  return JNI_OK;
 }

 jint InvokeInterface::destroyJavaVm(Jvm * vm) const {
  return vm->destroy();
 }
}