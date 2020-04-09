#include "fake-jni/jvm.h"

#include <stdexcept>

namespace FakeJni {
 static thread_local std::unique_ptr<JniEnv> javaOwnedEnv;

 jint InvokeInterface::attachCurrentThread(Jvm *vm, void **penv, void *args) const {
  auto env = JniEnv::getCurrentEnv();
  if ((env != nullptr && &env->vm != vm) || (env == nullptr && javaOwnedEnv.get()))
   return JNI_ERR;
  if (env == nullptr) {
   javaOwnedEnv = vm->createJniEnv();
   env = javaOwnedEnv.get();
   env->pushLocalFrame();
  }
  *penv = (void *)((JNIEnv *)env);
  return 0;
 }

 jint InvokeInterface::detachCurrentThread(Jvm *vm) const {
  auto env = JniEnv::getCurrentEnv();
  if (env == nullptr || &env->vm != vm)
   return JNI_ERR;
  if (javaOwnedEnv)
   javaOwnedEnv.reset();
  return 0;
 }

 jint InvokeInterface::attachCurrentThreadAsDaemon(Jvm *vm, void **penv, void *args) const {
  throw std::runtime_error("FATAL: 'JVMInvokeInterface_::AttachCurrentThread' is unimplemented!");
  return JNI_ERR;
 }
}