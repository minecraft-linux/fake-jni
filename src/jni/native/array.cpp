#include "fake-jni/jvm.h"
#include "fake-jni/array.h"

#include <cx/unsafe.h>

#include <stdexcept>

#define _NEW_ARRAY(array_type, ret_type)\
auto arr = new array_type(size);\
return (ret_type) env.createLocalReference(std::shared_ptr<JObject>(arr));

#define _GET_ARRAY_ELEMENTS(array_type)\
auto arr = std::dynamic_pointer_cast<array_type>(env.resolveReference(jarr)); \
if (copy) {\
 if (*copy) {\
  const auto len = arr->getSize();\
  auto data = new typename array_type::component[len];\
  for (JInt i = 0; i < len; i++) {\
   data[i] = (*arr)[i];\
  }\
  return data;\
 }\
}\
return arr->getArray();

//TODO ensure that a matching getArrayElements invocation occurred before freeing
#define _FREE_ARRAY_ELEMENTS(array_type)\
auto arr = std::dynamic_pointer_cast<array_type>(env.resolveReference(jarr)); \
const auto len = arr->getSize();\
if (mode == 0) {\
 for (JInt i = 0; i < len; i++) {\
  (*arr)[i] = elems[i];\
 }\
} else if (mode == JNI_COMMIT) {\
 for (JInt i = 0; i < len; i++) {\
  (*arr)[i] = elems[i];\
 }\
 delete[] elems;\
} else if (mode == JNI_ABORT) {\
 delete[] elems;\
} else {\
 throw std::runtime_error("FATAL: Invalid mode specified for release array elements!");\
}

//TODO JNI exception compliance
#define _GET_ARRAY_REGION(array_type)\
auto arr = std::dynamic_pointer_cast<array_type>(env.resolveReference(jarr)); \
const auto size = arr->getSize();\
if (0 > len || start + len > size) {\
 throw std::runtime_error("FATAL: Invalid array region requested!");\
}\
for (JInt i = start; i < start + len; i++) {\
 buf[i] = (*arr)[i];\
}

#define _SET_ARRAY_REGION(array_type)\
auto arr = std::dynamic_pointer_cast<array_type>(env.resolveReference(jarr)); \
const auto size = arr->getSize();\
if (0 > len || start + len > size) {\
 throw std::runtime_error("FATAL: Invalid array region requested!");\
}\
for (JInt i = start; i < start + len; i++) {\
 (*arr)[i] = buf[i];\
}

namespace FakeJni {
 void* NativeInterface::getPrimitiveArrayCritical(jarray jarr, jboolean * copy) const {
  auto arr = std::static_pointer_cast<JArray<void*, true>>(env.resolveReference(jarr));
  //We will never copy for now.
  if (copy != nullptr) {
      *copy = JNI_FALSE;
  }
  return arr->getArray();
 }

 void NativeInterface::releasePrimitiveArrayCritical(jarray jarr, void * elem, jint mode) const {
    //Nothing to release as we didn't copy
 }

 jsize NativeInterface::getArrayLength(jarray jarr) const {
  auto arr = std::dynamic_pointer_cast<JArrayBase>(env.resolveReference(jarr));
  return arr->getSize();
 }

 jobjectArray NativeInterface::newObjectArray(jsize size, jclass elementTypeRef, jobject initialElementRef) const {
  auto elementType = std::dynamic_pointer_cast<JClass>(env.resolveReference(elementTypeRef));
  auto initialElement = env.resolveReference(initialElementRef);
  auto arrayType = JArray<JObject>::getDescriptor();
  if (elementType) {
   auto tryArrayType = vm.findClass((std::string("[") + elementType->getSignature()).c_str());
   if (tryArrayType)
    arrayType = tryArrayType;
  }
  jvalue args[1];
  args[0].i = size;
  auto inst = std::dynamic_pointer_cast<JObjectArrayBase>(arrayType->newInstance(&vm, "(I)V", args));
  if (initialElement)
   inst->fillWithObject(initialElement);
  return (jobjectArray) env.createLocalReference(inst);
 }

 jobject NativeInterface::getObjectArrayElement(jobjectArray arrRef, jsize index) const {
  auto arr = std::dynamic_pointer_cast<JObjectArrayBase>(env.resolveReference(arrRef));
  return env.createLocalReference(arr->getObject(index));
 }

 void NativeInterface::setObjectArrayElement(jobjectArray arrRef, jsize index, jobject objRef) const {
  auto arr = std::dynamic_pointer_cast<JObjectArrayBase>(env.resolveReference(arrRef));
  auto obj = env.resolveReference(objRef);
  arr->setObject(index, obj);
 }

 jbooleanArray NativeInterface::newBooleanArray(jsize size) const {
  _NEW_ARRAY(JBooleanArray, jbooleanArray)
 }

 jbyteArray NativeInterface::newByteArray(jsize size) const {
  _NEW_ARRAY(JByteArray, jbyteArray)
 }

 jcharArray NativeInterface::newCharArray(jsize size) const {
  _NEW_ARRAY(JCharArray, jcharArray)
 }

 jshortArray NativeInterface::newShortArray(jsize size) const {
  _NEW_ARRAY(JShortArray, jshortArray)
 }

 jintArray NativeInterface::newIntArray(jsize size) const {
  _NEW_ARRAY(JIntArray, jintArray)
 }

 jlongArray NativeInterface::newLongArray(jsize size) const {
  _NEW_ARRAY(JLongArray, jlongArray)
 }

 jfloatArray NativeInterface::newFloatArray(jsize size) const {
  _NEW_ARRAY(JFloatArray, jfloatArray)
 }

 jdoubleArray NativeInterface::newDoubleArray(jsize size) const {
  _NEW_ARRAY(JDoubleArray, jdoubleArray)
 }

 jboolean * NativeInterface::getBooleanArrayElements(jbooleanArray jarr, jboolean * copy) const {
  _GET_ARRAY_ELEMENTS(JBooleanArray)
 }

 jbyte * NativeInterface::getByteArrayElements(jbyteArray jarr, jboolean * copy) const {
  _GET_ARRAY_ELEMENTS(JByteArray)
 }

 jchar * NativeInterface::getCharArrayElements(jcharArray jarr, jboolean * copy) const {
  _GET_ARRAY_ELEMENTS(JCharArray)
 }

 jshort * NativeInterface::getShortArrayElements(jshortArray jarr, jboolean * copy) const {
  _GET_ARRAY_ELEMENTS(JShortArray)
 }

 jint * NativeInterface::getIntArrayElements(jintArray jarr, jboolean * copy) const {
  _GET_ARRAY_ELEMENTS(JIntArray)
 }

 jlong * NativeInterface::getLongArrayElements(jlongArray jarr, jboolean * copy) const {
  _GET_ARRAY_ELEMENTS(JLongArray)
 }

 jfloat * NativeInterface::getFloatArrayElements(jfloatArray jarr, jboolean * copy) const {
  _GET_ARRAY_ELEMENTS(JFloatArray)
 }

 jdouble * NativeInterface::getDoubleArrayElements(jdoubleArray jarr, jboolean * copy) const {
  _GET_ARRAY_ELEMENTS(JDoubleArray)
 }

 void NativeInterface::releaseBooleanArrayElements(jbooleanArray jarr, jboolean * elems, jint mode) const {
  _FREE_ARRAY_ELEMENTS(JBooleanArray)
 }

 void NativeInterface::releaseByteArrayElements(jbyteArray jarr, jbyte * elems, jint mode) const {
  _FREE_ARRAY_ELEMENTS(JByteArray)
 }

 void NativeInterface::releaseCharArrayElements(jcharArray jarr, jchar * elems, jint mode) const {
  _FREE_ARRAY_ELEMENTS(JCharArray)
 }

 void NativeInterface::releaseShortArrayElements(jshortArray jarr, jshort * elems, jint mode) const {
  _FREE_ARRAY_ELEMENTS(JShortArray)
 }

 void NativeInterface::releaseIntArrayElements(jintArray jarr, jint * elems, jint mode) const {
  _FREE_ARRAY_ELEMENTS(JIntArray)
 }

 void NativeInterface::releaseLongArrayElements(jlongArray jarr, jlong * elems, jint mode) const {
  _FREE_ARRAY_ELEMENTS(JLongArray)
 }

 void NativeInterface::releaseFloatArrayElements(jfloatArray jarr, jfloat * elems, jint mode) const {
  _FREE_ARRAY_ELEMENTS(JFloatArray)
 }

 void NativeInterface::releaseDoubleArrayElements(jdoubleArray jarr, jdouble * elems, jint mode) const {
  _FREE_ARRAY_ELEMENTS(JDoubleArray)
 }

 void NativeInterface::getBooleanArrayRegion(jbooleanArray jarr, jsize start, jsize len, jboolean * buf) const {
  _GET_ARRAY_REGION(JBooleanArray)
 }

 void NativeInterface::getByteArrayRegion(jbyteArray jarr, jsize start, jsize len, jbyte * buf) const {
  _GET_ARRAY_REGION(JByteArray)
 }

 void NativeInterface::getCharArrayRegion(jcharArray jarr, jsize start, jsize len, jchar * buf) const {
  _GET_ARRAY_REGION(JCharArray)
 }

 void NativeInterface::getShortArrayRegion(jshortArray jarr, jsize start, jsize len, jshort * buf) const {
  _GET_ARRAY_REGION(JShortArray)
 }

 void NativeInterface::getIntArrayRegion(jintArray jarr, jsize start, jsize len, jint * buf) const {
  _GET_ARRAY_REGION(JIntArray)
 }

 void NativeInterface::getLongArrayRegion(jlongArray jarr, jsize start, jsize len, jlong * buf) const {
  _GET_ARRAY_REGION(JLongArray)
 }

 void NativeInterface::getFloatArrayRegion(jfloatArray jarr, jsize start, jsize len, jfloat * buf) const {
  _GET_ARRAY_REGION(JFloatArray)
 }

 void NativeInterface::getDoubleArrayRegion(jdoubleArray jarr, jsize start, jsize len, jdouble * buf) const {
  _GET_ARRAY_REGION(JDoubleArray)
 }

 void NativeInterface::setBooleanArrayRegion(jbooleanArray jarr, jsize start, jsize len, const jboolean * buf) const {
  _SET_ARRAY_REGION(JBooleanArray)
 }

 void NativeInterface::setByteArrayRegion(jbyteArray jarr, jsize start, jsize len, const jbyte * buf) const {
  _SET_ARRAY_REGION(JByteArray)
 }

 void NativeInterface::setCharArrayRegion(jcharArray jarr, jsize start, jsize len, const jchar * buf) const {
  _SET_ARRAY_REGION(JCharArray)
 }

 void NativeInterface::setShortArrayRegion(jshortArray jarr, jsize start, jsize len, const jshort * buf) const {
  _SET_ARRAY_REGION(JShortArray)
 }

 void NativeInterface::setIntArrayRegion(jintArray jarr, jsize start, jsize len, const jint * buf) const {
  _SET_ARRAY_REGION(JIntArray)
 }

 void NativeInterface::setLongArrayRegion(jlongArray jarr, jsize start, jsize len, const jlong * buf) const {
  _SET_ARRAY_REGION(JLongArray)
 }

 void NativeInterface::setFloatArrayRegion(jfloatArray jarr, jsize start, jsize len, const jfloat * buf) const {
  _SET_ARRAY_REGION(JFloatArray)
 }

 void NativeInterface::setDoubleArrayRegion(jdoubleArray jarr, jsize start, jsize len, const jdouble * buf) const {
  _SET_ARRAY_REGION(JDoubleArray)
 }
}