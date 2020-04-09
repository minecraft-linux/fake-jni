#include "fake-jni/jvm.h"

#include <stdexcept>

namespace FakeJni {
 jboolean NativeInterface::isSameObject(jobject ref1, jobject ref2) const {
  auto obj1 = env.resolveReference(ref1);
  auto obj2 = env.resolveReference(ref2);
  return (jboolean)(obj1 == obj2);
 }

 //TODO implement
 jobject NativeInterface::allocObject(jclass clazz) const {
  throw std::runtime_error("FATAL: 'JVMNativeInterface_::allocObject' is unimplemented!");
  return nullptr;
 }

 jobject NativeInterface::newObjectV(jclass const clazzRef, jmethodID mid, CX::va_list_t& args) const {
  auto clazz = std::dynamic_pointer_cast<JClass>(env.resolveReference(clazzRef));
  auto inst = clazz->newInstance(&vm, ((JMethodID *)mid)->getSignature(), args);
  return env.createLocalReference(inst);
 }

 jobject NativeInterface::newObjectA(jclass const clazzRef, jmethodID const mid, const jvalue * const args) const {
  auto clazz = std::dynamic_pointer_cast<JClass>(env.resolveReference(clazzRef));
  auto inst = clazz->newInstance(&vm, ((JMethodID *)mid)->getSignature(), args);
  return env.createLocalReference(inst);
 }

 jclass NativeInterface::getObjectClass(jobject const objRef) const {
  auto obj = env.resolveReference(objRef);
  return (jclass) env.createLocalReference(std::const_pointer_cast<JClass>(obj->getClassRef()));
 }

 jboolean NativeInterface::isInstanceOf(jobject const objRef, jclass const clazzRef) const {
  // TODO: incorrect implementation
  auto obj = env.resolveReference(objRef);
  auto clazz = env.resolveReference(clazzRef);
  return (unsigned char)(&obj->getClass() == ((JClass *)clazz.get()));
 }

 jclass NativeInterface::defineClass(const char *, jobject, const jbyte *, jsize) const {
  throw std::runtime_error("FATAL: 'JVMNativeInterface_::defineClass' is currently unsupported!");
  return nullptr;
 }

 jclass NativeInterface::findClass(const char * const name) const {
  return (jclass) env.createLocalReference(std::const_pointer_cast<JClass>(vm.findClass(name)));
 }

 jclass NativeInterface::getSuperclass(jclass clazzRef) const {
  auto clazz = std::dynamic_pointer_cast<JClass>(env.resolveReference(clazzRef));
  return (jclass) env.createLocalReference(std::const_pointer_cast<JClass>(clazz->parent));
 }

 jboolean NativeInterface::isAssignableFrom(jclass jderivedRef, jclass jbaseRef) const {
  const auto jobjDescriptor = &*JObject::getDescriptor();
  auto jderived = std::dynamic_pointer_cast<JClass>(env.resolveReference(jderivedRef));
  auto base = std::dynamic_pointer_cast<JClass>(env.resolveReference(jbaseRef));
  const JClass *derived = jderived.get();
  while (derived != jobjDescriptor) {
   if (derived == base.get()) {
    return JNI_TRUE;
   }
   derived = derived->parent.get();
  }
  return JNI_FALSE;
 }
}