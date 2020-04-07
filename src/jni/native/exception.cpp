#include "fake-jni/jvm.h"
#include "fake-jni/throwable.h"

#include <stdexcept>

namespace FakeJni {
 jint NativeInterface::throw_(jthrowable jthrow) const {
  const auto throwable = std::dynamic_pointer_cast<JThrowable>(env.resolveReference(jthrow));
  auto current = vm.getException();
  if (current) {
   throwable->addSuppressed(std::move(current));
   vm.clearException();
  }
  vm.throwException(throwable);
  return JNI_OK;
 }

 jint NativeInterface::throwNew(jclass jclazz, const char * message) const {
  const auto clazz = std::dynamic_pointer_cast<JClass>(env.resolveReference(jclazz));
  const JClass *derived = clazz.get();
  while (derived != &*JThrowable::getDescriptor() && derived != &*JObject::getDescriptor()) {
   derived = derived->parent.get();
  }
  if (derived != &*JThrowable::getDescriptor()) {
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
  auto jstrMessage = std::make_shared<JString>(message);
  auto jstrMessageRef = env.createLocalReference(jstrMessage); // TODO: Probably use magic reference here once they exist
  auto exception = constructor->invoke(env, clazz.get(), jstrMessageRef);
  env.deleteLocalReference(jstrMessageRef);
  vm.throwException(std::dynamic_pointer_cast<JThrowable>(env.resolveReference(exception)));
  env.deleteLocalReference(exception);
  return 0;
 }

 jthrowable NativeInterface::exceptionOccurred() const {
  return (jthrowable) env.createLocalReference(vm.getException());
 }

 void NativeInterface::exceptionDescribe() const {
  auto exception = vm.getException();
  if (exception) {
   exception->printStackTrace();
  }
 }

 void NativeInterface::exceptionClear() const {
  vm.clearException();
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