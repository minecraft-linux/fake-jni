#pragma once

#include "fake-jni/jvm.h"
#include "fake-jni/string.h"

#include <set>

//JNI java/lang/Throwable implementation
namespace FakeJni {
 class JThrowable : public JObject {
 private:
  std::set<std::shared_ptr<JThrowable>> suppressedExceptions;

  std::shared_ptr<JString> message;
  std::shared_ptr<JThrowable> cause;
  // JArray<JThrowable *> * suppressed;

 public:
  DEFINE_CLASS_NAME("java/lang/Throwable")

  JThrowable() noexcept;
  JThrowable(std::shared_ptr<JString> message);
  JThrowable(std::shared_ptr<JString> message, std::shared_ptr<JThrowable> cause);
  JThrowable(std::shared_ptr<JThrowable> cause);

  virtual ~JThrowable();

  void addSuppressed(std::shared_ptr<JThrowable> exception);
  const JThrowable * fillInStackTrace() const;
  std::shared_ptr<JThrowable> getCause() const;
  std::shared_ptr<JString> getMessage() const;
  // JArray<StackTraceElement> * getStackTrace();
  // const JArray<JThrowable *> * getSuppressed() const;
  const JThrowable * initCause(std::shared_ptr<JThrowable> cause);
  void printStackTrace() const;
  //void printStackTrace(PrintStream * s);
  //void printStackTrace(PrintWriter s);
  //void setStackTrace(JArray<StackTraceElement> * stackTrace)
  //TODO override toString once it's defined in JObject
  //void toString()
 };
}

_DEFINE_JNI_CONVERSION_OPERATOR(FakeJni::JThrowable, jthrowable)

DEFINE_JNI_TYPE(_jthrowable, "java/lang/Throwable")
DECLARE_NATIVE_TYPE(FakeJni::JThrowable)
