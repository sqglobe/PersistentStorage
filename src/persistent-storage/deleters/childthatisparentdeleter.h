#ifndef CHILDTHATISPARENTDELETER_H
#define CHILDTHATISPARENTDELETER_H

#include "defaultchilddeleter.h"
#include "parentsdeleter.h"

namespace prstorage {
template <typename K, typename V, typename P, typename C>
struct ChildThatIsParentDeleter
    : public DefaultChildDeleter<K, V, P, ParentsDeleter<K, V, C>> {
  using ParentDeleter = DefaultChildDeleter<K, V, P, ParentsDeleter<K, V, C>>;

  using ParentDeleter::DefaultChildDeleter;

  std::vector<typename ChildThatIsParentDeleter::ValueType> removeChilds(
      dbstl::db_multimap<typename ChildThatIsParentDeleter::ParentIdType,
                         typename ChildThatIsParentDeleter::ValueType>&
          secondary,
      const typename ChildThatIsParentDeleter::ParentType& parent)
  {
    std::vector<typename ChildThatIsParentDeleter::ValueType> deletedElements =
        ParentDeleter::removeChilds(secondary, parent);
    this->getChild()->parentRemoved(deletedElements);
    return deletedElements;
  }

  std::vector<typename ChildThatIsParentDeleter::ValueType> removeChilds(
      dbstl::db_multimap<typename ChildThatIsParentDeleter::ParentIdType,
                         typename ChildThatIsParentDeleter::ValueType>&
          secondary,
      const std::vector<typename ChildThatIsParentDeleter::ParentType>& parents)
  {
    std::vector<typename ChildThatIsParentDeleter::ValueType> deletedElements =
        ParentDeleter::removeChilds(secondary, parents);
    this->getChild()->parentRemoved(deletedElements);
    return deletedElements;
  }
};
}  // namespace prstorage
#endif  // CHILDTHATISPARENTDELETER_H
