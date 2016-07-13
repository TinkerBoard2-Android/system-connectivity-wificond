/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "net/nl80211_attribute.h"

#include <android-base/logging.h>

using std::string;
using std::vector;

namespace android {
namespace wificond {

// Explicit instantiation
template class NL80211Attr<uint8_t>;
template class NL80211Attr<uint16_t>;
template class NL80211Attr<uint32_t>;
template class NL80211Attr<vector<uint8_t>>;
template class NL80211Attr<string>;

// For BaseNL80211Attr
void BaseNL80211Attr::InitHeaderAndResize(int attribute_id,
                                          int payload_length) {
  data_.resize(NLA_HDRLEN + NLA_ALIGN(payload_length), 0);
  nlattr* header = reinterpret_cast<nlattr*>(data_.data());
  header->nla_type = attribute_id;
  header->nla_len = NLA_HDRLEN + payload_length;
}

int BaseNL80211Attr::GetAttributeId() const {
  const nlattr* header = reinterpret_cast<const nlattr*>(data_.data());
  return header->nla_type;
}

bool BaseNL80211Attr::IsValid() const {
  if (data_.size() < NLA_HDRLEN) {
    return false;
  }
  const nlattr* header = reinterpret_cast<const nlattr*>(data_.data());
  return NLA_ALIGN(header->nla_len) == data_.size();
}

const vector<uint8_t>& BaseNL80211Attr::GetConstData() const {
  return data_;
}

// For NL80211Attr<std::vector<uint8_t>>
NL80211Attr<vector<uint8_t>>::NL80211Attr(int id,
    const vector<uint8_t>& raw_buffer) {
  size_t size = raw_buffer.size();
  InitHeaderAndResize(id, size);
  memcpy(data_.data() + NLA_HDRLEN, raw_buffer.data(), raw_buffer.size());
}

NL80211Attr<vector<uint8_t>>::NL80211Attr(
    const vector<uint8_t>& data) {
  data_ = data;
}

vector<uint8_t> NL80211Attr<vector<uint8_t>>::GetValue() const {
  const nlattr* header = reinterpret_cast<const nlattr*>(data_.data());
  return vector<uint8_t>(
      data_.data() + NLA_HDRLEN,
      data_.data() + header->nla_len);
}

// For NL80211Attr<std::string>
NL80211Attr<string>::NL80211Attr(int id, const string& str) {
  size_t size = str.size();
  // This string is storaged as a null-terminated string.
  // Buffer is initialized with 0s so we only need to make a space for
  // the null terminator.
  InitHeaderAndResize(id, size + 1);
  char* storage = reinterpret_cast<char*>(data_.data() + NLA_HDRLEN);
  str.copy(storage, size);
}

NL80211Attr<string>::NL80211Attr(const vector<uint8_t>& data) {
  data_ = data;
}

string NL80211Attr<string>::GetValue() const {
  const nlattr* header = reinterpret_cast<const nlattr*>(data_.data());
  size_t str_length = header->nla_len - NLA_HDRLEN;
  // Remove trailing zeros.
  while (str_length > 0 &&
         *(data_.data() + NLA_HDRLEN + str_length - 1) == 0) {
    str_length--;
  }
  return string(reinterpret_cast<const char*>(data_.data() + NLA_HDRLEN),
                str_length);
}

// For NL80211NestedAttr
NL80211NestedAttr::NL80211NestedAttr(int id) {
  InitHeaderAndResize(id, 0);
}

NL80211NestedAttr::NL80211NestedAttr(const vector<uint8_t>& data) {
  data_ = data;
}

void NL80211NestedAttr::AddAttribute(const BaseNL80211Attr& attribute) {
  const vector<uint8_t>& append_data = attribute.GetConstData();
  // Append the data of |attribute| to |this|.
  data_.insert(data_.end(), append_data.begin(), append_data.end());
  nlattr* header = reinterpret_cast<nlattr*>(data_.data());
  // We don't need to worry about padding for nested attribute.
  // Because as long as all sub attributes have padding, the payload is aligned.
  header->nla_len += append_data.size();
}

void NL80211NestedAttr::AddFlagAttribute(int attribute_id) {
  // We only need to append a header for flag attribute.
  nlattr flag_header;
  flag_header.nla_type = attribute_id;
  flag_header.nla_len = NLA_HDRLEN;
  vector<uint8_t> append_data(
      reinterpret_cast<uint8_t*>(&flag_header),
      reinterpret_cast<uint8_t*>(&flag_header) + sizeof(nlattr));
  append_data.resize(NLA_HDRLEN, 0);
  // Append the data of |attribute| to |this|.
  data_.insert(data_.end(), append_data.begin(), append_data.end());

  nlattr* header = reinterpret_cast<nlattr*>(data_.data());
  header->nla_len += NLA_HDRLEN;
}

bool NL80211NestedAttr::HasAttribute(int id) const {
  return GetAttributeInternal(id, nullptr, nullptr);
}

bool NL80211NestedAttr::GetAttribute(int id,
    NL80211NestedAttr* attribute) const {
  uint8_t* start = nullptr;
  uint8_t* end = nullptr;
  if (!GetAttributeInternal(id, &start, &end) ||
      start == nullptr ||
      end == nullptr) {
    return false;
  }
  *attribute = NL80211NestedAttr(vector<uint8_t>(start, end));
  if (!attribute->IsValid()) {
    return false;
  }
  return true;
}

bool NL80211NestedAttr::GetAttributeInternal(int id,
                                             uint8_t** start,
                                             uint8_t** end) const {
  // Skip the top level attribute header.
  const uint8_t* ptr = data_.data() + NLA_HDRLEN;
  const uint8_t* end_ptr = data_.data() + data_.size();
  while (ptr + NLA_HDRLEN <= end_ptr) {
    const nlattr* header = reinterpret_cast<const nlattr*>(ptr);
    if (header->nla_type == id) {
      if (ptr + NLA_ALIGN(header->nla_len) > end_ptr) {
        LOG(ERROR) << "Failed to get attribute: broken nl80211 atrribute.";
        return false;
      }
      if (start != nullptr && end != nullptr) {
        *start = const_cast<uint8_t*>(ptr);
        *end = const_cast<uint8_t*>(ptr + NLA_ALIGN(header->nla_len));
      }
      return true;
    }
    ptr += NLA_ALIGN(header->nla_len);
  }
  return false;
}

}  // namespace wificond
}  // namespace android