#include "fake-jni/jvm.h"
#include "fake-jni/array.h"

#define DEFINE_PRIMITIVE_ARRAY(array_type) \
template<>\
const std::shared_ptr<const FakeJni::JClass> &FakeJni::JArray<array_type, true>::getDescriptor() noexcept {\
 static const std::shared_ptr<const FakeJni::JClass> descriptor (new FakeJni::JClass {\
  FakeJni::JClass::PUBLIC,\
  FakeJni::_CX::JClassBreeder<FakeJni::JArray<array_type>> {\
   {FakeJni::Constructor<JArray<array_type>, JInt> {}},\
   {FakeJni::Field<&JArray<array_type>::length> {}, "length"}\
  }\
 });\
 return descriptor;\
}

namespace FakeJni {

 DEFINE_PRIMITIVE_ARRAY(JBoolean)
 DEFINE_PRIMITIVE_ARRAY(JByte)
 DEFINE_PRIMITIVE_ARRAY(JChar)
 DEFINE_PRIMITIVE_ARRAY(JShort)
 DEFINE_PRIMITIVE_ARRAY(JInt)
 DEFINE_PRIMITIVE_ARRAY(JFloat)
 DEFINE_PRIMITIVE_ARRAY(JLong)
 DEFINE_PRIMITIVE_ARRAY(JDouble)

}

