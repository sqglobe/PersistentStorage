#ifndef DEFAULTCHILDDELETER_H
#define DEFAULTCHILDDELETER_H

#include <dbstl_map.h>
#include <algorithm>
#include <vector>

namespace prstorage {
template <typename K, typename V, typename P, typename D>
struct DefaultChildDeleter : public D {
  using ParentType = P;
  using ParentIdType = decltype(get_id(std::declval<P>()));
  using ParentDeleter = D;

  template <typename... Args>
  DefaultChildDeleter(Args&&... args) : D(std::forward<Args>(args)...) {}

  std::vector<typename DefaultChildDeleter::ValueType> operator()(
      dbstl::db_multimap<ParentIdType, typename DefaultChildDeleter::ValueType>&
          secondary,
      const ParentType& parent) {
    std::vector<typename DefaultChildDeleter::ValueType> deletedElements;
    auto [begin, end] = secondary.equal_range(get_id(parent));
    std::transform(begin, end, std::back_inserter(deletedElements),
                   [](const auto& el) { return el.second; });

    auto count = secondary.erase(get_id(parent));
    return deletedElements;
  }

  std::vector<typename DefaultChildDeleter::ValueType> operator()(
      dbstl::db_multimap<ParentIdType, typename DefaultChildDeleter::ValueType>&
          secondary,
      const std::vector<ParentType>& parents) {
    std::vector<typename DefaultChildDeleter::ValueType> deletedElements;
    for (auto parent : parents) {
      auto [begin, end] = secondary.equal_range(get_id(parent));
      std::transform(begin, end, std::back_inserter(deletedElements),
                     [](const auto& el) { return el.second; });
      secondary.erase(get_id(parent));
    }
    return deletedElements;
  }
};
}  // namespace prstorage

#endif  // DEFAULTCHILDDELETER_H
