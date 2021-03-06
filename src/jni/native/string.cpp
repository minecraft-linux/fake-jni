#include "fake-jni/jvm.h"
#include "fake-jni/string.h"

#include <cx/unsafe.h>

#include <stdexcept>
#include <assert.h>

namespace FakeJni {
 jstring NativeInterface::newString(const jchar * chars, jsize size) const {
  return (jstring) env.createLocalReference(std::make_shared<JString>(chars, size));
 }

 jsize NativeInterface::getStringLength(jstring strRef) const {
  auto str = std::dynamic_pointer_cast<JString>(env.resolveReference(strRef));
  return str->getLength();
 }

 jchar * NativeInterface::getStringChars(jstring strRef, jboolean * copy) const {
  auto str = std::dynamic_pointer_cast<JString>(env.resolveReference(strRef));
  if (copy) {
   *copy = JNI_TRUE;
  }
  //TODO do JChar strings need to be null terminated?
  auto c_str = new JChar[str->getSize()];
  memcpy(c_str, str->getArray(), (size_t)str->getLength());
  return c_str;
 }

 void NativeInterface::releaseStringChars(jstring, const jchar * chars) const {
  //TODO check that a matching getStringChars invocation occured before deleting
  delete[] chars;
 }

 jstring NativeInterface::newStringUTF(const char * c_str) const {
  return (jstring) env.createLocalReference(std::make_shared<JString>(c_str));
 }

 jsize NativeInterface::getStringUTFLength(jstring strRef) const {
  auto str = std::dynamic_pointer_cast<JString>(env.resolveReference(strRef));
  return str->getLength();
 }

 char * NativeInterface::getStringUTFChars(jstring strRef, jboolean * copy) const {
  auto str = std::dynamic_pointer_cast<JString>(env.resolveReference(strRef));
  if (copy) {
   *copy = JNI_TRUE;
  }
  const auto len = str->getLength();
  auto c_str = new char[len + 1];
  memcpy(c_str, (char *)str->getArray(), (size_t)len);
  c_str[len] = '\0';
  return c_str;
 }

 void NativeInterface::releaseStringUTFChars(jstring, const char * c_str) const {
  delete[] c_str;
 }

 void NativeInterface::getStringRegion(jstring strRef, jsize start, jsize len, jchar * buf) const {
  auto str = std::dynamic_pointer_cast<JString>(env.resolveReference(strRef));
  auto data = str->getArray();
  const auto slen = str->getLength();
  if (0 > len || start + len > slen) {
   //TODO JNI exception compliance
   throw std::runtime_error("FATAL: Invalid string region requested!");
  }
  for (JInt i = start; i < start + len; i++) {
   buf[i] = data[i];
  }
 }

 void NativeInterface::getStringUTFRegion(jstring strRef, jsize start, jsize len, char * buf) const {
  auto str = std::dynamic_pointer_cast<JString>(env.resolveReference(strRef));
  auto data = (char *)str->getArray();
  const auto slen = str->getLength();
  if (0 > len || start + len > slen) {
   //TODO JNI exception compliance
   throw std::runtime_error("FATAL: Invalid UTF string region requested!");
  }
  for (JInt i = start; i < start + len; i++) {
   buf[i] = data[i];
  }
 }

//TODO implement
 jchar* NativeInterface::getStringCritical(jstring, jboolean *) const {
  throw std::runtime_error("FATAL: 'JVMNativeInterface_::getStringCritical' is unimplemented!");
  return nullptr;
 }

//TODO implement
 void NativeInterface::releaseStringCritical(jstring, const jchar *) const {
  throw std::runtime_error("FATAL: 'JVMNativeInterface_::releaseStringCritical' is unimplemented!");
 }
}