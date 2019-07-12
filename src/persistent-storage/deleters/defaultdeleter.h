#ifndef DEFAULTDELETER_H
#define DEFAULTDELETER_H

#include <dbstl_map.h>
#include <optional>

namespace prstorage {
template <typename K, typename V>
struct DefaultDeleter {
  using KeyType = K;
  using ValueType = V;

  std::optional<ValueType> operator()(
      dbstl::db_map<KeyType, ValueType>& elements,
      const KeyType& id)
  {
    if (auto iter = elements.find(id); iter != elements.end()) {
      auto elem = *iter;
      elements.erase(iter);
      return {elem.second};
    }
    return {};
  }
};
}  // namespace prstorage
#endif  // DEFAULTDELETER_H
