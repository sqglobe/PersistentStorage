#ifndef CHILDTHATISPARENTDELETER_H
#define CHILDTHATISPARENTDELETER_H

#include "defaultchilddeleter.h"
#include "parentsdeleter.h"
template <typename K, typename V, typename P, typename C>
struct ChildThatIsParentDeleter: public DefaultChildDeleter<K, V, P, ParentsDeleter<K, V, C>>{
    using ParentDeleter = DefaultChildDeleter<K, V, P, ParentsDeleter<K, V, C>>;

    using ParentDeleter::DefaultChildDeleter;

    std::vector<typename ChildThatIsParentDeleter::ValueType> operator()(
            dbstl::db_multimap<typename ChildThatIsParentDeleter::ParentIdType, typename ChildThatIsParentDeleter::ValueType> &secondary,
            const typename ChildThatIsParentDeleter::ParentType &parent)
    {
        std::vector<typename ChildThatIsParentDeleter::ValueType> deletedElements = ParentDeleter::operator()(secondary, parent);
        this->getChild()->parentRemoved(deletedElements);
        return deletedElements;
    }

    std::vector<typename ChildThatIsParentDeleter::ValueType> operator()(
            dbstl::db_multimap<typename ChildThatIsParentDeleter::ParentIdType, typename ChildThatIsParentDeleter::ValueType> &secondary,
            const std::vector<typename ChildThatIsParentDeleter::ParentType> &parents)
    {
        std::vector<typename ChildThatIsParentDeleter::ValueType> deletedElements = ParentDeleter::operator()(secondary, parents);
        this->getChild()->parentRemoved(deletedElements);
        return deletedElements;
    }
};

#endif // CHILDTHATISPARENTDELETER_H
