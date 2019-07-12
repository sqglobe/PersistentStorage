#ifndef CHILDSTORAGE_H
#define CHILDSTORAGE_H

#include <db_cxx.h>
#include <dbstl_map.h>
#include <optional>
#include "storage.h"

#include "persistent-storage/deleters/defaultchilddeleter.h"
#include "persistent-storage/deleters/defaultdeleter.h"

namespace prstorage {
template <
    typename Element,
    typename Parent,
    typename Marshaller,
    typename Watcher,
    typename TxManager = DefaultTransactionManager,
    typename Deleter = DefaultChildDeleter<
        decltype(get_id(std::declval<Element>())),
        Element,
        Parent,
        DefaultDeleter<decltype(get_id(std::declval<Element>())), Element>>>
class ChildStorage
    : public Storage<Element, Marshaller, Watcher, TxManager, Deleter> {
 public:
  using ParentContainer =
      Storage<Element, Marshaller, Watcher, TxManager, Deleter>;
  using ParentElementId = decltype(get_id(std::declval<Parent>()));

 public:
  ChildStorage(Db* db,
               Db* secondary,
               DbEnv* env,
               Deleter&& deleter = Deleter());

 public:
  void parentRemoved(const Parent& parent);
  void parentRemoved(const std::vector<Parent>& parents);

 private:
  Db* mSecondaryDb;
  dbstl::db_multimap<ParentElementId, Element> mSecondaryKeys;
};

}  // namespace prstorage

template <typename Element,
          typename Parent,
          typename Marshaller,
          typename Watcher,
          typename TxManager,
          typename Deleter>
prstorage::
    ChildStorage<Element, Parent, Marshaller, Watcher, TxManager, Deleter>::
        ChildStorage(Db* db, Db* secondary, DbEnv* env, Deleter&& deleter) :
    ChildStorage<Element, Parent, Marshaller, Watcher, TxManager, Deleter>::
        ParentContainer(db, env, std::move(deleter)),
    mSecondaryDb(secondary), mSecondaryKeys(secondary, env)
{
}

template <typename Element,
          typename Parent,
          typename Marshaller,
          typename Watcher,
          typename TxManager,
          typename Deleter>
void prstorage::
    ChildStorage<Element, Parent, Marshaller, Watcher, TxManager, Deleter>::
        parentRemoved(const Parent& parent)
{
  auto deletedElements = this->getDeleter()(mSecondaryKeys, parent);
  std::for_each(std::cbegin(deletedElements), std::cend(deletedElements),
                [this](const Element& element) {
                  ParentContainer::watcher_type::elementRemoved(element);
                });
}

template <typename Element,
          typename Parent,
          typename Marshaller,
          typename Watcher,
          typename TxManager,
          typename Deleter>
void prstorage::
    ChildStorage<Element, Parent, Marshaller, Watcher, TxManager, Deleter>::
        parentRemoved(const std::vector<Parent>& parents)
{
  auto deletedElements = this->getDeleter()(mSecondaryKeys, parents);
  std::for_each(std::cbegin(deletedElements), std::cend(deletedElements),
                [this](const Element& element) {
                  ParentContainer::watcher_type::elementRemoved(element);
                });
}

#endif  // CHILDSTORAGE_H
