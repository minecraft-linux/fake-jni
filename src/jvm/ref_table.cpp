#include "fake-jni/jvm.h"

namespace FakeJni {
 JniReferenceTable::JniReferenceTable(size_t size) {
  resizeFrame(size);
 }

 void JniReferenceTable::resizeFrame(size_t size) {
  if (size < references.size())
   throw std::runtime_error("FATAL: Growing local frame down is not supported");
  auto oldSize = references.size();
  references.resize(size);
  referencesNextIndex.reserve(size + 1);
  if (referencesNextIndex.empty())
   referencesNextIndex.push_back(1);
  for (auto i = 1 + oldSize; i < size; i++)
   referencesNextIndex.push_back(i + 1);
  if (size > oldSize)
   referencesNextIndex.push_back(0);
  referencesNextIndex[lastReferenceIndex] = 1 + oldSize;
  lastReferenceIndex = size;
 }

 void JniReferenceTable::ensureSize(size_t size) {
  if (size > references.size())
   resizeFrame(size);
 }

 size_t JniReferenceTable::reserveReference() {
  auto ret = referencesNextIndex[0];
  referencesNextIndex[0] = referencesNextIndex[ret];
  referencesNextIndex[ret] = 0;
  return ret;
 }

 void JniReferenceTable::returnReference(size_t index) {
  referencesNextIndex[lastReferenceIndex] = index;
  lastReferenceIndex = index;
 }

}