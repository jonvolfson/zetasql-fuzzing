//
// Copyright 2019 ZetaSQL Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef ZETASQL_PUBLIC_VALUE_CONTENT_H_
#define ZETASQL_PUBLIC_VALUE_CONTENT_H_

#include <type_traits>

#include <cstdint>

namespace zetasql {

class SimpleType;

// ValueContent class is a typeless represention of zetasql::Value content
// (Value, in comparison, also stores a reference to zetasql::Type instance):
// 1) ValueContent is not stored in Value directly, but serves as a value's
//  content exchange container in a contract between Value and Type for
//  conveying value content from Type to Value class. This helps to decouple
//  Type and Value: ValueContent doesn't depend on either of them and thus
//  allows avoiding circular-dependency between Value (which references Type)
//  and Type (which defines Value content).
// 2) Value class stores value's content within its space together with other
//  Value's fields (like, type identifier) using different layouts depending on
//  whether Value belongs to simple type or not. Simple types are those builtin
//  types that can be represented with just a TypeKind, with no parameters (in
//  ZetaSQL type's classes hierarchy, simple types are represented by
//  SimpleType class).
// 3) This class should only be used inside Value and inside Type subclasses.
// 4) ValueContent provides 8 bytes of value's content and is_null flag.
// 5) For builtin simple types, an extra four bytes of storage is available in
//  Value because the type is stored as an TypeKind enum rather than a Type*
//  pointer.  This extra storage (in simple_type_extended_content_) is usable
//  only in SimpleType.
class ValueContent {
 private:
  template <typename T>
  struct Storage {
    union {
      int64_t base_value;
      T value;
    };
  };

 public:
  template <class T>
  static constexpr bool IsTypeSupported() {
    return std::is_trivially_copyable<T>::value &&
           sizeof(Storage<T>) == sizeof(content_);
  }

  template <class T>
  std::enable_if_t<IsTypeSupported<T>(), void> set(T value) {
    content<T>()->value = value;
  }

  template <class T>
  std::enable_if_t<IsTypeSupported<T>(), void> get(T* value) const {
    *value = content<T>()->value;
  }

  template <class T>
  std::enable_if_t<IsTypeSupported<T>(), T> GetAs() const {
    return content<T>()->value;
  }

  bool is_null() const { return is_null_; }

 private:
  friend class SimpleType;
  friend class Value;

  template <typename T>
  Storage<T>* content() {
    static_assert(IsTypeSupported<T>());
    return reinterpret_cast<Storage<T>*>(&content_);
  }

  template <typename T>
  const Storage<T>* content() const {
    static_assert(IsTypeSupported<T>());
    return reinterpret_cast<const Storage<T>*>(&content_);
  }

  static constexpr ValueContent NullValue() {
    return ValueContent(/*value=*/0, /*extended_value=*/0, /*is_null=*/true);
  }

  constexpr ValueContent(int64_t value, int32_t extended_value, bool is_null)
      : content_(value),
        simple_type_extended_content_(extended_value),
        is_null_(is_null) {}

  // Main content of the value that all types use
  int64_t content_;

  // Field below can be used only by simple types (SimpleType).
  int32_t simple_type_extended_content_;

  // Whether value is null
  bool is_null_;
};

}  // namespace zetasql

#endif  // ZETASQL_PUBLIC_VALUE_CONTENT_H_
