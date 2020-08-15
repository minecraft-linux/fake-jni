#pragma once

#include "fake-jni/jvm.h"

namespace FakeJni {
 //Utility base, not a registered fake-jni type
 template<typename T, bool Primitive = _CX::IsPrimitive<T>::value>
 class JArray;

 class JArrayBase : public JObject {
 public:
  template<typename T>
  operator T() const;

  virtual JInt getSize() const = 0;
 };

 //Immutable array base
 template<typename Component, typename Base>
 class JArrayImpl : public Base {
 protected:
  JInt length;
  Component * array;

 private:
  static JInt boundsCheck(JInt len);

 public:
  template<typename C>
  operator C() const;

  JArrayImpl(std::initializer_list<Component>);
  explicit JArrayImpl(const JArrayImpl<Component, Base> & array);
  explicit JArrayImpl(JInt size);
  explicit JArrayImpl(Component * arr, JInt length);
  explicit JArrayImpl(std::vector<Component> arr);
  JArrayImpl() : JArrayImpl(0) {}
  virtual ~JArrayImpl();

  inline JInt getSize() const final {
   return length;
  }

  inline const Component* getArray() const {
   return array;
  }

  inline Component* getArray() {
   return array;
  }

  const Component& operator[](JInt i) const;

  JArrayImpl<Component, Base>& operator=(const JArrayImpl<Component, Base> & arr);
  Component& operator[](JInt i);
 };

 //Mutable array implementation
 template<typename T>
 class JArray<T, true> : public JArrayImpl<T, JArrayBase> {
 public:
  using component = T;

  using JArrayImpl<T, JArrayBase>::JArrayImpl;

  //fake-jni metadata
  static constexpr const auto name = _CX::JniTypeBase<JArray<T>>::signature;
  static const std::shared_ptr<const JClass> & getDescriptor() noexcept;
  virtual const JClass & getClass() const noexcept override {
   return *getDescriptor();
  }
  virtual std::shared_ptr<const JClass> getClassRef() const noexcept override {
   return getDescriptor();
  }
 };

 struct JObjectArrayBase : JArrayBase {
  virtual std::shared_ptr<JObject> getObject(JInt index) = 0;
  virtual void setObject(JInt index, std::shared_ptr<JObject> obj) = 0;
  virtual void fillWithObject(std::shared_ptr<JObject> obj) = 0;
 };

 template<typename T>
 class JArray<T, false> : public JArrayImpl<std::shared_ptr<T>, JObjectArrayBase> {
 public:
  using component = std::shared_ptr<T>;

  using JArrayImpl<std::shared_ptr<T>, JObjectArrayBase>::JArrayImpl;

  std::shared_ptr<JObject> getObject(JInt index) override {
   return (*this)[index];
  }
  void setObject(JInt index, std::shared_ptr<JObject> obj) override {
   (*this)[index] = std::dynamic_pointer_cast<T>(obj);
  }
  void fillWithObject(std::shared_ptr<JObject> obj) override {
   auto array = JArrayImpl<std::shared_ptr<T>, JObjectArrayBase>::getArray();
   auto cobj = std::dynamic_pointer_cast<T>(obj);
   for (auto it = JArrayImpl<std::shared_ptr<T>, JObjectArrayBase>::getSize() - 1; it >= 0; --it)
    array[it] = cobj;
  }

  //fake-jni metadata
  static constexpr const auto name = _CX::JniTypeBase<JArray<T>>::signature;
  static const std::shared_ptr<const JClass> & getDescriptor() noexcept;
  virtual const JClass & getClass() const noexcept override {
   return *getDescriptor();
  }
  virtual std::shared_ptr<const JClass> getClassRef() const noexcept override {
   return getDescriptor();
  }
 };

 namespace _CX {
 template<typename T>
 class JniTypeBase<JArray<T>> {
 private:
  static constexpr const char arrayPrefix[] = "[";
 public:
  static constexpr const bool isRegisteredType = true;
  static constexpr const bool isClass = true;
  static constexpr const auto signature = CX::Concat<arrayPrefix, JniTypeBase<T>::signature>::result;
  static constexpr const bool hasComplexHierarchy = false;
 };
 }

 //ArrayBase template members
 template<typename T>
 JArrayBase::operator T() const {
  using component_t = typename CX::ComponentTypeResolver<T>::type;
  constexpr const auto
   downcast = __is_base_of(JArrayBase, component_t),
   upcast = __is_base_of(component_t, JArrayBase),
   jnicast = CX::MatchAny<component_t, _jobject, _jarray>::value;
  static_assert(
   downcast || upcast || jnicast,
   "JArray<T> can only be upcast, downcast, or converted to _jobject or _jarray!"
  );
  auto ptr = const_cast<JArrayBase *>(this);
  if constexpr(upcast || downcast) {
   return (T&)*ptr;
  } else if constexpr (jnicast) {
   return CX::union_cast<T>(this);
  }
 }

 //Immutable JArray template members
 template<typename T, typename Base>
 template<typename C>
 JArrayImpl<T, Base>::operator C() const {
  return JArrayBase::operator C();
 }

 template<typename T, typename Base>
 inline JInt JArrayImpl<T, Base>::boundsCheck(JInt len) {
  if (len < 0) {
   throw std::out_of_range("FATAL: Arrays cannot have a negative size!");
  }
  return len;
 }

 template<typename T, typename Base>
 JArrayImpl<T, Base>::JArrayImpl(std::initializer_list<T> list) : JArrayImpl((JInt)list.size()) {
  size_t i = 0;
  for (auto element : list) {
   array[i++] = element;
  }
 }

 template<typename T, typename Base>
 JArrayImpl<T, Base>::JArrayImpl(const JArrayImpl<T, Base> & array) : JArrayImpl(array.length) {
  memcpy(array, array.array, length);
 }

 template<typename T, typename Base>
 JArrayImpl<T, Base>::JArrayImpl(const JInt size) :
  length(boundsCheck(size)),
  array(new T[length])
 {}

 template<typename T, typename Base>
 JArrayImpl<T, Base>::JArrayImpl(T * const arr, const JInt length) : JArrayImpl(length) {
  for (JInt i = 0; i < length; i++) {
   array[i] = arr[i];
  }
 }

 template<typename T, typename Base>
 JArrayImpl<T, Base>::JArrayImpl(std::vector<T> vec) : JArrayImpl((JInt) vec.size()) {
  size_t i = 0;
  for (auto element : vec) {
   array[i++] = element;
  }
 }

 template<typename T, typename Base>
 JArrayImpl<T, Base>::~JArrayImpl() {
  delete[] array;
 }

 template<typename T, typename Base>
 const T& JArrayImpl<T, Base>::operator[](const JInt i) const {
  return array[i];
 }

 //Mutable JArray template members
 template<typename T, typename Base>
 T& JArrayImpl<T, Base>::operator[](const JInt i) {
  if (i > length) {
   throw std::out_of_range("Requested index out of range!");
  }
  return array[i];
 }

 template<typename T, typename Base>
 JArrayImpl<T, Base>& JArrayImpl<T, Base>::operator=(const JArrayImpl<T, Base> &arr) {
  auto& ref = const_cast<JArrayImpl<T, Base>&>(*this);
  if (ref.length != arr.length) {
   ref.length = arr.length;
   delete[] ref.array;
   ref.array = new T[ref.length];
  }
  for (JInt i = 0; i < ref.length; i++) {
   ref.array[i] = arr.array[i];
  }
  return ref;
 }

 template<typename T>
 const std::shared_ptr<const FakeJni::JClass> &FakeJni::JArray<T, false>::getDescriptor() noexcept {
  static const std::shared_ptr<const FakeJni::JClass> descriptor (new FakeJni::JClass {
   FakeJni::JClass::PUBLIC,
   FakeJni::_CX::JClassBreeder<FakeJni::JArray<T>> {
    {FakeJni::Constructor<JArray<T>, JInt> {}},
    {FakeJni::Field<&JArray<T>::length> {}, "length"}
   }
  });
  return descriptor;
 }


 using JBooleanArray = JArray<JBoolean>;
 using JByteArray = JArray<JByte>;
 using JCharArray = JArray<JChar>;
 using JShortArray = JArray<JShort>;
 using JIntArray = JArray<JInt>;
 using JFloatArray = JArray<JFloat>;
 using JLongArray = JArray<JLong>;
 using JDoubleArray = JArray<JDouble>;
}

//Clean up internal macros
#undef _DEFINE_NATIVE_ARRAY