#include "fake-jni/jvm.h"
#include "fake-jni/throwable.h"

#include <stdexcept>

namespace FakeJni {
 jint NativeInterface::throw_(jthrowable jthrow) const {
  JThrowable * throwable = *jthrow;
  jthrowable current = exceptionOccurred();
  if (current) {
   throwable->addSuppressed(*current);
   vm.clearException();
  }
  vm.throwException(*throwable);
  return JNI_OK;
 }

 jint NativeInterface::throwNew(jclass jclazz, const char * message) const {
  const auto clazz = ((JClass *)*jclazz);
  if (!isAssignableFrom(jclazz, *JThrowable::getDescriptor())) {
   throw std::runtime_error(
    "FATAL: Requested exception class '"
     + std::string(clazz->getName())
     + "' does not extend 'java/lang/Throwable'!"
   );
  }
  auto constructor = clazz->getMethod("Ljava/lang/String;", "<init>");
  if (!constructor) {
   throw std::runtime_error(
    "FATAL: Requested exception class '"
     + std::string(clazz->getName())
     + "' does not expose a constructor matching the prototype: '<init>(Ljava/lang/String;)'!"
   );
  }
  //clean up string eventually
  auto jstrMessage = new JString(message);
  vm.throwException(constructor->invoke(&vm, clazz, jstrMessage));
  return 0;
 }

 jthrowable NativeInterface::exceptionOccurred() const {
  return vm.getException();
 }

 void NativeInterface::exceptionDescribe() const {
  auto exception = vm.getException();
  if (exception) {
   ((JThrowable *)*exception)->printStackTrace();
  }
 }

 void NativeInterface::exceptionClear() const {
  JThrowable * exception = *vm.getException();
  vm.clearException();
  delete exception;
 }

 jboolean NativeInterface::exceptionCheck() const {
  return (jboolean)(vm.getException() ? JNI_TRUE : JNI_FALSE);
 }

 void NativeInterface::fatalError(const char * message) const {
  vm.fatalError(message);
 }

 jint NativeInterface::pushLocalFrame(jint size) const {
  env.pushLocalFrame(size);
  return 0;
 }

 jobject NativeInterface::popLocalFrame(jobject move) const {
  std::shared_ptr<JObject> moveRef;
  if (move != nullptr)
   moveRef = env.resolveReference(move);
  env.popLocalFrame();
  if (moveRef)
   return env.createLocalReference(moveRef);
  return nullptr;
 }
}