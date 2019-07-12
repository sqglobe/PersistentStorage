#ifndef STORAGE_H
#define STORAGE_H

#include <algorithm>
#include <functional>

#include <db_cxx.h>
#include <dbstl_map.h>
#include <optional>
#include "defaulttransactionmanager.h"
#include "persistent-storage/deleters/defaultdeleter.h"
#include "persistent-storage/wrappers/transparentcontainerelementwrapper.h"
/*
 * Needed realisation of functions:
 *  std::string get_id(ConstElement element );
 *
 */

/**
 * Шаблонный класс, который служит для хранения различных объектов: диалогов,
 * контактов и пр. Обеспечивает добавление, удаление элементов. Поиск по
 * идентификатору, поиск по предикату и пр. Так же обеспечивается возможность
 * добавить обработчики событий о добавлении/удалении/изменении элементов.
 *
 * Обеспечивает способ получения класса-обертки элемента. При создании обертки
 * хранимое значение элемента копируется, так что изменения выполненные через
 * прокси-класс не затрагивают сам контейнер. Обертка так же реализует
 * прозрачный доступ к элементам к элементам оборачиваемого объекта.
 * Оборачиваемое значение сохраняется только при вызове метода обертки.
 *
 * Необходимость создания такого прокси-класса - транзакционность изменений
 * объектов, для сохранения изменений нет надобности хранить изменяемое значение
 * и класс-контейнер
 *
 * @param Element - тип хранимого элемента
 * @param ConstElement - константный тип хранимого элемента (особенно актуально
 * при применении указателей) для передачи, например, в наблюдатели при
 * наступлении события изменения элемента.
 *
 * Для хранимых элементов необходимо реализовать свободную функцию
 *        std::string get_id(ConstElement element );
 * эта функция должна возвращать идентификатор объекта element.
 *
 * Пример Marshaller:
 * class ContactMarshaller {
 *   public:
 *     static void restore(std::shared_ptr<Contact>& elem, const void* src);
 *     static size_t size(const std::shared_ptr<Contact>& element);
 *     static void store(void* dest, const std::shared_ptr<Contact>& elem);
 *  };
 *
 * Пример Watcher:
 * class TestWatcher{
 *   void elementAdded(const Element &);
 *   void elementRemoved(const Element &);
 *   void elementUpdated(const Element &);
 * };
 */

namespace prstorage {
template <
    typename Element,
    typename Marshaller,
    typename Watcher,
    typename TxManager = DefaultTransactionManager,
    typename Deleter =
        DefaultDeleter<decltype(get_id(std::declval<Element>())), Element>>
class Storage : public std::enable_shared_from_this<
                    Storage<Element, Marshaller, Watcher, TxManager, Deleter>>,
                public Watcher {
 public:
  using element = Element;
  using watcher_type = Watcher;
  using key = decltype(get_id(std::declval<Element>()));
  using wrapper_type = TransparentContainerElementWrapper<
      Storage<Element, Marshaller, Watcher, TxManager, Deleter>>;
  using TransactionManager = TxManager;

 public:
  Storage(Db* db, DbEnv* env, Deleter&& deleter = Deleter());
  explicit Storage(Db* db, Deleter&& deleter = Deleter());
  explicit Storage(Deleter&& deleter = Deleter());

 public:
  /**
   * @brief Добавляется элемент
   * @param elem элемент, который необходимо добавить
   */
  bool add(const element& elem);

  /**
   * @brief удаляет элемент по его идентификатору.
   * @param id идентификатор объекта, который необходимо удалить
   * @return true - если удаление прошло успешно, в противном случае - false.
   */
  bool remove(const key& id);

  /**
   * @brief Выполняет только обновление элемента путем перезаписи
   * Для успешного выполнения функции у объекта не должен изменяться
   * идентификатор.
   * @param elem - элемент, который необходимо обновить
   * @return true, если элемент находится в контейнере и обновлен, false - в
   * противном случае.
   */
  bool strictUpdate(const element& elem);

  /**
   * @brief Выполняет обновление, если элемент присутствует в хранилище, в
   * противном случае элемент добавляется.
   * @param elem элемент, который нужно обновить/добавить
   */
  void update(const element& elem);

  /**
   * @brief Возвращает класс обертку для указанного идентификатора.
   * Обертка инкапсулирует внутри контейнер и копию изменяемого объекта.
   * Для сохранения изменений достаточно выполнить метод обертки.
   * @param id идентификатор объекта, для которого необходимо создать обертку.
   * @return класс-обертку.
   * @throws std::out_of_range если указанный объект не найден
   */
  wrapper_type wrapper(const key& id);

 public:
  element get(const key& id) const;
  bool has(const key& id) const;
  std::vector<element> getAllElements() const;
  int size() const noexcept;

  std::vector<element> get_if(std::function<bool(const element&)> p) const;

 protected:
  element find(std::function<bool(const element&)> is) const;
  Deleter& getDeleter();

 private:
  mutable dbstl::db_map<key, element> mElements;
  mutable DbEnv* mEnv;
  Deleter mDeleter;
};
}  // namespace prstorage
/*-----------------------------------------------------------------------------------------------------*/

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename TxManager,
          typename Deleter>
prstorage::Storage<Element, Marshaller, Watcher, TxManager, Deleter>::Storage(
    Db* db,
    DbEnv* env,
    Deleter&& deleter) :
    mElements(db, env),
    mEnv(env), mDeleter(std::forward<Deleter>(deleter)) {
  auto inst = dbstl::DbstlElemTraits<Element>::instance();
  inst->set_size_function(&Marshaller::size);
  inst->set_copy_function(&Marshaller::store);
  inst->set_restore_function(&Marshaller::restore);
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename TxManager,
          typename Deleter>
prstorage::Storage<Element, Marshaller, Watcher, TxManager, Deleter>::Storage(
    Db* db,
    Deleter&& deleter) :
    Storage(db, db->get_env(), std::forward<Deleter>(deleter)) {}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename TxManager,
          typename Deleter>
prstorage::Storage<Element, Marshaller, Watcher, TxManager, Deleter>::Storage(
    Deleter&& deleter) :
    Storage(nullptr, nullptr, std::forward<Deleter>(deleter)) {}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename TxManager,
          typename Deleter>
bool prstorage::Storage<Element, Marshaller, Watcher, TxManager, Deleter>::add(
    const Storage::element& elem) {
  TransactionManager manager(mEnv);
  if (auto [it, res] = mElements.insert(std::make_pair(get_id(elem), elem));
      res) {
    manager.commit();
    watcher_type::elementAdded(elem);
    return true;
  }
  return false;
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename TxManager,
          typename Deleter>
bool prstorage::Storage<Element, Marshaller, Watcher, TxManager, Deleter>::
    remove(const key& id) {
  TransactionManager manager(mEnv);
  if (auto res = mDeleter(mElements, id); res) {
    manager.commit();
    watcher_type::elementRemoved(*res);
    return true;
  }
  return false;
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename TxManager,
          typename Deleter>
bool prstorage::Storage<Element, Marshaller, Watcher, TxManager, Deleter>::
    strictUpdate(const Storage::element& elem) {
  TransactionManager manager(mEnv);
  if (auto iter = mElements.find(get_id(elem)); iter != mElements.end()) {
    *iter = std::make_pair(get_id(elem), elem);
    manager.commit();
    watcher_type::elementUpdated(elem);
    return true;
  }
  return false;
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename TxManager,
          typename Deleter>
void prstorage::Storage<Element, Marshaller, Watcher, TxManager, Deleter>::
    update(const Storage::element& elem) {
  TransactionManager manager(mEnv);
  mElements[get_id(elem)] = elem;
  manager.commit();
  watcher_type::elementUpdated(elem);
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename TxManager,
          typename Deleter>
typename prstorage::Storage<Element, Marshaller, Watcher, TxManager, Deleter>::
    wrapper_type
    prstorage::Storage<Element, Marshaller, Watcher, TxManager, Deleter>::
        wrapper(const key& id) {
  return wrapper_type(this->shared_from_this(), get(id));
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename TxManager,
          typename Deleter>
typename prstorage::Storage<Element, Marshaller, Watcher, TxManager, Deleter>::
    element
    prstorage::Storage<Element, Marshaller, Watcher, TxManager, Deleter>::get(
        const key& id) const {
  if (auto iter = mElements.find(id, true); iter != mElements.end()) {
    return (*iter).second;
  }
  throw std::range_error("not found element");
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename TxManager,
          typename Deleter>
bool prstorage::Storage<Element, Marshaller, Watcher, TxManager, Deleter>::has(
    const key& id) const {
  auto it = mElements.find(id, true);
  return mElements.end() != it;
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename TxManager,
          typename Deleter>
std::vector<
    typename prstorage::
        Storage<Element, Marshaller, Watcher, TxManager, Deleter>::element>
prstorage::Storage<Element, Marshaller, Watcher, TxManager, Deleter>::
    getAllElements() const {
  std::vector<element> res;
  std::transform(
      mElements.begin(dbstl::ReadModifyWriteOption::no_read_modify_write(),
                      true),
      mElements.end(), std::back_inserter(res),
      [](const auto& elem) { return elem.second; });
  return res;
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename TxManager,
          typename Deleter>
int prstorage::Storage<Element, Marshaller, Watcher, TxManager, Deleter>::size()
    const noexcept {
  return mElements.size();
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename TxManager,
          typename Deleter>
typename prstorage::Storage<Element, Marshaller, Watcher, TxManager, Deleter>::
    element
    prstorage::Storage<Element, Marshaller, Watcher, TxManager, Deleter>::find(
        std::function<bool(const Storage::element&)> is) const {
  auto it = std::find_if(
      mElements.begin(dbstl::ReadModifyWriteOption::no_read_modify_write(),
                      true),
      mElements.end(), is);

  if (mElements.cend() != it) {
    return *it;
  }

  throw std::range_error("not found element");
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename TxManager,
          typename Deleter>
Deleter& prstorage::Storage<Element, Marshaller, Watcher, TxManager, Deleter>::
    getDeleter() {
  return mDeleter;
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename TxManager,
          typename Deleter>
std::vector<
    typename prstorage::
        Storage<Element, Marshaller, Watcher, TxManager, Deleter>::element>
prstorage::Storage<Element, Marshaller, Watcher, TxManager, Deleter>::get_if(
    std::function<bool(const element&)> p) const {
  std::vector<element> res;
  for (auto it = mElements.begin(
           dbstl::ReadModifyWriteOption::no_read_modify_write(), true);
       it != mElements.end(); ++it) {
    auto val = *it;
    if (p(val.second)) {
      res.push_back(val.second);
    }
  }
  return res;
}

#endif  // STORAGE_H
