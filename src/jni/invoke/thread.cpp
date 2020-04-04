#include "fake-jni/jvm.h"

#include <stdexcept>

namespace FakeJni {
 jint InvokeInterface::attachCurrentThread(Jvm *vm, void **penv, void *args) const {
  if (penv)
   *penv = (void *)((JNIEnv *)&vm->getJniEnv());
  return 0;
 }

 jint InvokeInterface::detachCurrentThread(Jvm *vm) const {
  return JNI_ERR;
 }

 jint InvokeInterface::attachCurrentThreadAsDaemon(Jvm *vm, void **penv, void *args) const {
  throw std::runtime_error("FATAL: 'JVMInvokeInterface_::AttachCurrentThread' is unimplemented!");
  return JNI_ERR;
 }
}