#include "fake-jni/jvm.h"

namespace FakeJni {
 JniReferenceTable::JniReferenceTable(size_t size, size_t start) :
   startIndex(start)
 {
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
  if (referencesNextIndex[0] == 0) {
   resizeFrame(references.size() * 2);
  }
  auto ret = referencesNextIndex[0];
  referencesNextIndex[0] = referencesNextIndex[ret];
  if (referencesNextIndex[ret] == 0)
   lastReferenceIndex = 0;
  referencesNextIndex[ret] = 0;
  return ret;
 }

 void JniReferenceTable::returnReference(size_t index) {
  referencesNextIndex[lastReferenceIndex] = index;
  lastReferenceIndex = index;
 }

 size_t JniReferenceTable::createReference(std::shared_ptr<JObject> ref) {
  auto ret = reserveReference();
  references[ret] = std::move(ref);
  return startIndex + ret;
 }

 void JniReferenceTable::deleteReference(size_t index) {
  index -= startIndex;
  if (index < 0 || index > references.size())
   throw std::runtime_error("FATAL: Invalid out-of-bounds reference index");
  if (!references[index])
   throw std::runtime_error("FATAL: Invalid empty reference");
  references[index].reset();
  returnReference(index);
 }

 std::shared_ptr<JObject> JniReferenceTable::getReference(size_t index) {
  index -= startIndex;
  if (index < 0 || index > references.size())
   return std::shared_ptr<JObject>();
  return references[index];
 }
}