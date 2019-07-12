#ifndef PARENTSDELETER_H
#define PARENTSDELETER_H

#include "defaultdeleter.h"
#include <memory>

template <typename K, typename V, typename C>
class ParentsDeleter: public DefaultDeleter<K, V> {
public:

  using ParentDeleter = DefaultDeleter<K, V>;
  using ChildContainer = C;

public:
    ParentsDeleter(std::shared_ptr<ChildContainer> child): mChild(std::move(child)) {}

public:
  std::optional<typename ParentDeleter::ValueType> operator()(dbstl::db_map<typename ParentDeleter::KeyType, typename ParentDeleter::ValueType>& elements, const typename ParentDeleter::KeyType& id) {
     auto res = ParentDeleter::operator()(elements, id);
     if(res){
         mChild->parentRemoved(*res);
     }
     return res;
  }
protected:
  std::shared_ptr<ChildContainer> getChild() const{
      return mChild;
  }

private:
  std::shared_ptr<ChildContainer> mChild;
};

#endif // PARENTSDELETER_H
