#ifndef STORE_H
#define STORE_H

#include <algorithm>
#include <functional>

#include <db_cxx.h>
#include <dbstl_map.h>
#include <optional>
#include "PersistentStorage/wrappers/transparentcontainerelementwrapper.h"

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
 */

template <typename K, typename V>
struct DefaultDeleter {
  std::optional<V> operator()(dbstl::db_map<K, V>& elements, const K& id) {
    if (auto iter = elements.find(id); iter != elements.end()) {
      auto elem = *iter;
      elements.erase(iter);
      return {elem};
    }
    return {};
  }
};

template <
    typename Element,
    typename Marshaller,
    typename Watcher,
    typename Deleter =
        DefaultDeleter<decltype(get_id(std::declval<Element>())), Element>>
class Store : public std::enable_shared_from_this<
                  Store<Element, Marshaller, Watcher, Deleter>>,
              public Watcher {
 public:
  using element = Element;
  using watcher_type = Watcher;
  using key = decltype(get_id(std::declval<Element>()));
  using wrapper_type = TransparentContainerElementWrapper<
      Store<Element, Marshaller, Watcher, Deleter>>;

 public:
  Store(Db* db, ENV* env, Deleter&& deleter = Deleter());
  explicit Store(Db* db, Deleter&& deleter = Deleter());
  Store(Deleter&& deleter = Deleter());

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

 protected:
  dbstl::db_map<key, element> mElements;
  Deleter mDeleter;
};

/*-----------------------------------------------------------------------------------------------------*/

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename Deleter>
Store<Element, Marshaller, Watcher, Deleter>::Store(Db* db,
                                                    ENV* env,
                                                    Deleter&& deleter) :
    mElements(db, env),
    mDeleter(std::forward(deleter)) {
  auto inst = dbstl::DbstlElemTraits<Element>::instance();
  inst->set_size_function(&Marshaller::size);
  inst->set_copy_function(&Marshaller::store);
  inst->set_restore_function(&Marshaller::restore);
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename Deleter>
Store<Element, Marshaller, Watcher, Deleter>::Store(Db* db, Deleter&& deleter) :
    Store(db, db->get_ENV(), std::forward(deleter)) {}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename Deleter>
Store<Element, Marshaller, Watcher, Deleter>::Store(Deleter&& deleter) :
    Store(nullptr, nullptr, std::forward(deleter)) {}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename Deleter>
bool Store<Element, Marshaller, Watcher, Deleter>::add(
    const Store::element& elem) {
  if (auto [it, res] = mElements.insert(std::make_pair(get_id(elem), elem));
      res) {
    watcher_type::elementAdded(elem);
    return true;
  }

  return false;
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename Deleter>
bool Store<Element, Marshaller, Watcher, Deleter>::remove(const key& id) {
  if (auto res = mDeleter(mElements, id); res) {
    watcher_type::elementRemoved(*res);
    return true;
  }
  return false;
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename Deleter>
bool Store<Element, Marshaller, Watcher, Deleter>::strictUpdate(
    const Store::element& elem) {
  if (auto iter = mElements.find(get_id(elem)); iter != mElements.end()) {
    iter = elem;
    watcher_type::elementUpdated(elem);
    return true;
  }
  return false;
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename Deleter>
void Store<Element, Marshaller, Watcher, Deleter>::update(
    const Store::element& elem) {
  mElements[get_id(elem)] = elem;
  watcher_type::elementUpdated(elem);
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename Deleter>
typename Store<Element, Marshaller, Watcher, Deleter>::wrapper_type
Store<Element, Marshaller, Watcher, Deleter>::wrapper(const key& id) {
  return wrapper_type(this->shared_from_this(), get(id));
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename Deleter>
typename Store<Element, Marshaller, Watcher, Deleter>::element
Store<Element, Marshaller, Watcher, Deleter>::get(const key& id) const {
  if (auto iter = mElements.find(id, true); iter != mElements.end()) {
    return *iter;
  }
  throw std::range_error("not found element");
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename Deleter>
bool Store<Element, Marshaller, Watcher, Deleter>::has(const key& id) const {
  auto it = mElements.find(id, true);
  return mElements.cend() != it;
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename Deleter>
std::vector<typename Store<Element, Marshaller, Watcher, Deleter>::element>
Store<Element, Marshaller, Watcher, Deleter>::getAllElements() const {
  std::vector<element> res;
  std::copy(mElements.begin(
                dbstl::ReadModifyWriteOption::no_read_modify_write(), true),
            mElements.end(), std::back_inserter(res));
  return res;
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename Deleter>
int Store<Element, Marshaller, Watcher, Deleter>::size() const noexcept {
  return mElements.size();
}

template <typename Element,
          typename Marshaller,
          typename Watcher,
          typename Deleter>
typename Store<Element, Marshaller, Watcher, Deleter>::element
Store<Element, Marshaller, Watcher, Deleter>::find(
    std::function<bool(const Store::element&)> is) const {
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
          typename Deleter>
std::vector<typename Store<Element, Marshaller, Watcher, Deleter>::element>
Store<Element, Marshaller, Watcher, Deleter>::get_if(
    std::function<bool(const element&)> p) const {
  std::vector<element> res;
  std::copy_if(mElements.begin(
                   dbstl::ReadModifyWriteOption::no_read_modify_write(), true),
               mElements.end(), std::back_inserter(res), p);
  return res;
}

#endif  // STORE_H
