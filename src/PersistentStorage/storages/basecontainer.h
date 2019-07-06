#ifndef BASECONTAINER_H
#define BASECONTAINER_H

#include <algorithm>
#include <functional>
#include <iterator>
#include <list>
#include <memory>
#include <mutex>
#include <vector>
#include "PersistentStorage/wrappers/transparentcontainerelementwrapper.h"

/*
 * Needed realisation of functions:
 *  std::string get_id(ConstElement element );
 *
 */

template <typename C>
class ChangeWatcher;

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
 */
template <typename Key, typename Element, typename Watcher>
class BaseContainer
    : public std::enable_shared_from_this<BaseContainer<Key, Element, Watcher>>,
      public Watcher {
 public:
  using element = Element;
  using watcher_type = Watcher;
  using key = Key;
  using wrapper_type =
      TransparentContainerElementWrapper<BaseContainer<Key, Element, Watcher>>;

 public:
  /**
   * @brief Добавляется элемент
   * @param elem элемент, который необходимо добавить
   */
  void add(const element& elem);

  /**
   * @brief удаляет элемент по его идентификатору.
   * @param id идентификатор объекта, который необходимо удалить
   * @return true - если удаление прошло успешно, в противном случае - false.
   */
  bool remove(const key& id);

  /**
   * @brief Удаляет указанный элемент
   * @param elem - элемент, который необходимо удалить
   * @return  true - если удаление прошло успешно, в противном случае - false.
   */
  bool remove(const element& elem);

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
  bool has(const std::string& id) const;
  std::vector<element> getAllElements() const;
  int size() const noexcept;

  std::vector<element> get_if(std::function<bool(const element&)> p) const;

 protected:
  element find(std::function<bool(const element&)> is) const;

 protected:
  std::vector<element> mElements;
  mutable std::recursive_mutex mMutex;
};

/*-----------------------------------------------------------------------------------------------------*/

template <typename Key, typename Element, typename Watcher>
void BaseContainer<Key, Element, Watcher>::add(
    const BaseContainer::element& elem) {
  std::lock_guard<std::recursive_mutex> guard(mMutex);
  mElements.push_back(elem);
  watcher_type::elementAdded(elem);
}

template <typename Key, typename Element, typename Watcher>
bool BaseContainer<Key, Element, Watcher>::remove(const key& id) {
  std::lock_guard<std::recursive_mutex> guard(mMutex);
  auto it = find(id);
  if (it != mElements.cend()) {
    auto elem = *it;
    mElements.erase(it);
    watcher_type::elementRemoved(elem);
    return true;
  }
  return false;
}

template <typename Key, typename Element, typename Watcher>
bool BaseContainer<Key, Element, Watcher>::remove(
    const BaseContainer::element& elem) {
  return remove(get_id(elem));
}

template <typename Key, typename Element, typename Watcher>
bool BaseContainer<Key, Element, Watcher>::strictUpdate(
    const BaseContainer::element& elem) {
  std::lock_guard<std::recursive_mutex> guard(mMutex);
  auto it = find(get_id(elem));
  if (it != mElements.cend()) {
    mElements[std::distance(mElements.cbegin(), it)] = elem;
    watcher_type::elementUpdated(elem);
    return true;
  }

  return false;
}

template <typename Key, typename Element, typename Watcher>
void BaseContainer<Key, Element, Watcher>::update(
    const BaseContainer::element& elem) {
  std::lock_guard<std::recursive_mutex> guard(mMutex);
  auto it = find(get_id(elem));
  if (it != mElements.cend()) {
    mElements[std::distance(mElements.cbegin(), it)] = elem;
  } else {
    mElements.push_back(elem);
  }
  watcher_type::elementUpdated(elem);
}

template <typename Key, typename Element, typename Watcher>
typename BaseContainer<Key, Element, Watcher>::wrapper_type
BaseContainer<Key, Element, Watcher>::wrapper(const key& id) {
  std::lock_guard<std::recursive_mutex> guard(mMutex);
  return wrapper_type(this->shared_from_this(), get(id));
}

template <typename Key, typename Element, typename Watcher>
typename BaseContainer<Key, Element, Watcher>::element
BaseContainer<Key, Element, Watcher>::get(const key& id) const {
  std::lock_guard<std::recursive_mutex> guard(mMutex);
  auto pos = posOf(id);
  return at(pos);
}

template <typename Key, typename Element, typename Watcher>
bool BaseContainer<Key, Element, Watcher>::has(const std::string& id) const {
  std::lock_guard<std::recursive_mutex> guard(mMutex);
  auto it = find(id);
  return mElements.cend() != it;
}

template <typename Key, typename Element, typename Watcher>
std::vector<typename BaseContainer<Key, Element, Watcher>::element>
BaseContainer<Key, Element, Watcher>::getAllElements() const {
  std::lock_guard<std::recursive_mutex> guard(mMutex);
  std::vector<element> res;
  std::copy(mElements.cbegin(), mElements.cend(), std::back_inserter(res));
  return res;
}

template <typename Key, typename Element, typename Watcher>
int BaseContainer<Key, Element, Watcher>::size() const noexcept {
  std::lock_guard<std::recursive_mutex> guard(mMutex);
  return mElements.size();
}

template <typename Key, typename Element, typename Watcher>
typename BaseContainer<Key, Element, Watcher>::element
BaseContainer<Key, Element, Watcher>::find(
    std::function<bool(const BaseContainer::element&)> is) const {
  std::lock_guard<std::recursive_mutex> guard(mMutex);
  auto it = std::find_if(mElements.cbegin(), mElements.cend(), is);

  if (mElements.cend() != it) {
    return *it;
  }

  throw std::range_error("not found element");
}

template <typename Key, typename Element, typename Watcher>
std::vector<typename BaseContainer<Key, Element, Watcher>::element>
BaseContainer<Key, Element, Watcher>::get_if(
    std::function<bool(const element&)> p) const {
  std::lock_guard<std::recursive_mutex> guard(mMutex);
  std::vector<typename BaseContainer<Key, Element, Watcher>::element> res;
  std::copy_if(std::cbegin(mElements), std::cend(mElements),
               std::back_inserter(res), p);
  return res;
}

#endif  // BASECONTAINER_H
