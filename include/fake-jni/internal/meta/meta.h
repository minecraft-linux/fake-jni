#pragma once

#include <cx/idioms.h>
#include <cx/classes.h>

namespace FakeJni {
 //fake-jni integral types
 using JVoid = void;
 using JBoolean = jboolean;
 using JByte = jbyte;
 using JChar = jchar;
 using JShort = jshort;
 using JInt = jint;
 using JFloat = jfloat;
 using JLong = jlong;
 using JDouble = jdouble;

 //Wrapper template for constructor detection and registration
 template<typename T, typename... Args>
 class Constructor {
 public:
  constexpr Constructor() noexcept {
   static_assert(CX::HasConstructor<T, Args...>::value, "Tried to register non-existent constructor!");
  }

  [[gnu::always_inline]]
  inline static T * construct(Args... args) {
   return new T(args...);
  }
 };

 template<auto F, typename T = decltype(F), auto = CX::IsFunction<T>::value>
 struct Function;

 template<auto F>
 struct Function<F, decltype(F), nullptr> {
  static constexpr const auto function = F;

  constexpr Function() noexcept = default;
 };

 template<auto F, typename T, typename R, typename... Args>
 struct Function<F, R (T::*)(Args...), true> : Function<F, decltype(F), nullptr> {
  constexpr Function() noexcept {
   static_assert(
    !CX::IsVirtualFunction<F>::value,
    "Virtual functions cannot be registered with fake-jni! If you intend to register an overload for a native base "
    "type, you must register a function with the exact same name and signature as the target function in the base."
   );
  }
 };

 template<auto F, typename R, typename... Args>
 struct Function<F, R (*)(Args...), true> : Function<F, decltype(F), nullptr> {
  using Function<F, decltype(F), nullptr>::Function;
 };

 template<auto F, typename R, typename... Args>
 struct Function<F, R (&)(Args...), true> : Function<F, decltype(F), nullptr> {
  using Function<F, decltype(F), nullptr>::Function;
 };

 template<auto F, typename R, typename... Args>
 struct Function<F, R (Args...), true> : Function<F, decltype(F), nullptr> {
  using Function<F, decltype(F), nullptr>::Function;
 };

 template<auto F, typename T = decltype(F), auto = CX::IsField<T>::value>
 struct Field;

 template<auto F>
 struct Field<F, decltype(F), nullptr> {
  static constexpr const auto field = F;

  constexpr Field() noexcept = default;
 };

 template<auto F, typename C, typename T>
 struct Field<F, T C::*, true> : Field<F, decltype(F), nullptr> {
  using Field<F, decltype(F), nullptr>::Field;
 };

 template<auto F, typename T>
 struct Field<F, T *, true> : Field<F, decltype(F), nullptr> {
  using Field<F, decltype(F), nullptr>::Field;
 };
}

//fake-jni type metadata templates
namespace FakeJni::_CX {
 using CX::ComponentTypeResolver;
 using CX::ExplicitCastGenerator;
 using CX::IsTemplateTemplate;
 using CX::TemplateIsSame;

 //Stub type for generating pointer-to-member types
 class AnyClass;

 //JNI Type metadata template
 template<typename>
 class JniTypeBase {
 public:
  static constexpr const bool isRegisteredType = false;
  static constexpr const bool isClass = false;
  static constexpr const char signature[] = "[INVALID_TYPE]";
  static constexpr const bool hasComplexHierarchy = false;
 };

 //Native array metadata template
 template<typename T>
 class JniArrayTypeBase;

 //Strip pointers off of JniTypeBase specializations and instantiations
 template<typename T>
 class JniTypeBase<T*> : public JniTypeBase<typename ComponentTypeResolver<T*>::type> {};

 //Strip references off of JniTypeBase specializations and instantiations
 template<typename T>
 class JniTypeBase<T&> : public JniTypeBase<typename ComponentTypeResolver<T&>::type> {};

 //Strip const qualifications off of JniTypeBase specializations and instantiations
 template<typename T>
 class JniTypeBase<const T> : public JniTypeBase<typename ComponentTypeResolver<const T>::type> {};

 //'cast' alias detection idiom
 //Negative case
 template<typename T, typename V = void>
 class CastDefined : public CX::false_type {
 public:
  [[gnu::always_inline]]
  inline static void assertAliasCorrectness() {
   //Always false
   static_assert(
    CX::IsSame<V, void>::value,
    "Complex JNI types must define a 'cast' alias!"
   );
  }
 };

 //Positive case
 template<typename T>
 class CastDefined<T, CX::void_t<typename T::cast>> : public CX::true_type {
 private:
  template<typename>
  class TemplateTemplateDecomposer {};

  template<template<typename...> typename T1, typename... Args>
  class TemplateTemplateDecomposer<T1<Args...>> {
  public:
   [[gnu::always_inline]]
   inline static constexpr bool verifyParameters() {
    static_assert(
     TemplateIsSame<T1, ExplicitCastGenerator>::value,
     "Illegal type for 'cast' alias, should be 'ExplicitCastGenerator<...>'!"
    );
    static_assert(
     ((JniTypeBase<Args>::isRegisteredType && JniTypeBase<Args>::isClass) && ...),
     "You may only use registered JNI / FakeJni classes in an explicit casting route!"
    );
    return true;
   }
  };

 public:
  using cast = typename T::cast;

  [[gnu::always_inline]]
  inline static void assertAliasCorrectness() {
   static_assert(
    IsTemplateTemplate<cast>::value && TemplateTemplateDecomposer<cast>::verifyParameters(),
    "Illegal type for 'cast' alias, should be 'ExplicitCastGenerator<...>'!"
   );
  }
 };

 template<typename T, typename = void>
 class BaseDefined : public CX::false_type {};

 template<typename T>
 class BaseDefined<T, CX::void_a<T::base>>;
}