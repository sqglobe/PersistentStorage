#ifndef SIMPLESTORAGE_H
#define SIMPLESTORAGE_H

#include <algorithm>
#include <functional>

#include <db_cxx.h>
#include <dbstl_map.h>
#include <optional>
#include "defaulttransactionmanager.h"
#include "persistent-storage/deleters/defaultdeleter.h"
#include "persistent-storage/wrappers/transparentcontainerelementwrapper.h"

namespace prstorage {
/**
 * Шаблонный класс-коонтейнер, который обчеспечивает сохранение данных на диск.
 *
 * Данные хранятся в виде ключ/значение. Типом значения является параметр
 * шаблона Element, а для получения ключа должна быть определена свободная
 * функция get_id(const Element&).
 *
 * Параметры шаблона:
 * @tparam Element тип хранимого элемента
 * @tparam Marshaller тип, который обеспечивает функцилонал для
 * маршалинга/демаршалинга элементов Element в массив байт и обратно

 * @tparam Deleter отвечает за удаление элемента из контейнера, пример -
 * DefaultDeleter
 *
 * Пример Marshaller:
 * class ContactMarshaller {
 *   public:
 *     static void restore(std::shared_ptr<Contact>& elem, const void* src);
 *     static size_t size(const std::shared_ptr<Contact>& element);
 *     static void store(void* dest, const std::shared_ptr<Contact>& elem);
 *  };
 *
 * При добавлении/удалении/обновлении элемента, контейнер использует функции,
 * которые предоставляет Watcher, для уведомления о событии. Необходимо
 * определение в этом классе следующих функций: class TestWatcher{ protected:
 *   void elementAdded(const Element &);
 *   void elementRemoved(const Element &);
 *   void elementUpdated(const Element &);
 * };
 */
template <
    typename Element,
    typename Marshaller,
    typename Deleter =
        DefaultDeleter<decltype(get_id(std::declval<Element>())), Element>>
class SimpleStorage {
 public:
  using element = Element;
  using key = decltype(get_id(std::declval<Element>()));

 public:
  /**
   * @brief Контруктор класса
   * @param db экземпляр базы данных Berkeley DB, в котором хранятся элементы
   * контейнера
   * @param env экземпляр окружения для db
   * @param deleter объект, который выполняет удаление элементов из БД
   */
  SimpleStorage(Db* db, DbEnv* env, Deleter&& deleter = Deleter());

  /**
   * @brief Контруктор класса
   * @param db экземпляр базы данных Berkeley DB, в котором хранятся элементы
   * контейнера
   * @param deleter объект, который выполняет удаление элементов из БД
   */
  explicit SimpleStorage(Db* db, Deleter&& deleter = Deleter());

  /**
   * @brief Контруктор класса
   * @param deleter объект, который выполняет удаление элементов из БД
   */
  explicit SimpleStorage(Deleter&& deleter = Deleter());

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

 public:
  /**
   * @brief Возвращает объект по заданному ключу, в случае неудачного
   * поиска выбрасывается исключение std::range_error
   * @param id ключ, по которому осуществляется поиск
   * @return элемент, который соответствует ключчу
   * @throws std::range_error при отсутствии ключа
   */
  element get(const key& id) const;

  /**
   * @brief Проверяет наличие элемента с указанным ключем в хранилище
   * @param id ключ, для которого проверяется наличие элемента
   * @return флаг наличия элемента в хранилище
   */
  bool has(const key& id) const;

  /**
   * @brief Извлекает из хранилища все элементы и возвращает в виде std::vector
   * @return std::vector с элементами массива
   */
  std::vector<element> getAllElements() const;

  /**
   * @brief Возвращает количество элементов в хранилище
   * @return количество элементов в храниоище
   */
  int size() const noexcept;

  /**
   * @brief Возвращает список элементов, которые удовлетворяют предикату
   * @param p предикат, который принимает ссылку на хранимый элемент и
   * возвращает: true - элемент должен быть добавлен в результирующее множество
   * false - нет
   * @return элементы, удовлетворяющие предикату
   */
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

template <typename Element, typename Marshaller, typename Deleter>
prstorage::SimpleStorage<Element, Marshaller, Deleter>::SimpleStorage(
    Db* db,
    DbEnv* env,
    Deleter&& deleter) :
    mElements(db, env),
    mEnv(env), mDeleter(std::forward<Deleter>(deleter))
{
  auto inst = dbstl::DbstlElemTraits<Element>::instance();
  inst->set_size_function(&Marshaller::size);
  inst->set_copy_function(&Marshaller::store);
  inst->set_restore_function(&Marshaller::restore);
}

template <typename Element, typename Marshaller, typename Deleter>
prstorage::SimpleStorage<Element, Marshaller, Deleter>::SimpleStorage(
    Db* db,
    Deleter&& deleter) :
    SimpleStorage(db, db->get_env(), std::forward<Deleter>(deleter))
{
}

template <typename Element, typename Marshaller, typename Deleter>
prstorage::SimpleStorage<Element, Marshaller, Deleter>::SimpleStorage(
    Deleter&& deleter) :
    SimpleStorage(nullptr, nullptr, std::forward<Deleter>(deleter))
{
}

template <typename Element, typename Marshaller, typename Deleter>
bool prstorage::SimpleStorage<Element, Marshaller, Deleter>::add(
    const SimpleStorage::element& elem)
{
  if (auto [it, res] = mElements.insert(std::make_pair(get_id(elem), elem));
      res) {
    return true;
  }
  return false;
}

template <typename Element, typename Marshaller, typename Deleter>
bool prstorage::SimpleStorage<Element, Marshaller, Deleter>::remove(
    const key& id)
{
  if (auto res = mDeleter(mElements, id); res) {
    return true;
  }
  return false;
}

template <typename Element, typename Marshaller, typename Deleter>
bool prstorage::SimpleStorage<Element, Marshaller, Deleter>::strictUpdate(
    const SimpleStorage::element& elem)
{
  if (auto iter = mElements.find(get_id(elem)); iter != mElements.end()) {
    *iter = std::make_pair(get_id(elem), elem);
    return true;
  }
  return false;
}

template <typename Element, typename Marshaller, typename Deleter>
void prstorage::SimpleStorage<Element, Marshaller, Deleter>::update(
    const SimpleStorage::element& elem)
{
  mElements[get_id(elem)] = elem;
}

template <typename Element, typename Marshaller, typename Deleter>
typename prstorage::SimpleStorage<Element, Marshaller, Deleter>::element
prstorage::SimpleStorage<Element, Marshaller, Deleter>::get(const key& id) const
{
  if (auto iter = mElements.find(id, true); iter != mElements.end()) {
    return (*iter).second;
  }
  throw std::range_error("not found element");
}

template <typename Element, typename Marshaller, typename Deleter>
bool prstorage::SimpleStorage<Element, Marshaller, Deleter>::has(
    const key& id) const
{
  auto it = mElements.find(id, true);
  return mElements.end() != it;
}

template <typename Element, typename Marshaller, typename Deleter>
std::vector<
    typename prstorage::SimpleStorage<Element, Marshaller, Deleter>::element>
prstorage::SimpleStorage<Element, Marshaller, Deleter>::getAllElements() const
{
  std::vector<element> res;
  std::transform(
      mElements.begin(dbstl::ReadModifyWriteOption::no_read_modify_write(),
                      true),
      mElements.end(), std::back_inserter(res),
      [](const auto& elem) { return elem.second; });
  return res;
}

template <typename Element, typename Marshaller, typename Deleter>
int prstorage::SimpleStorage<Element, Marshaller, Deleter>::size() const
    noexcept
{
  return mElements.size();
}

template <typename Element, typename Marshaller, typename Deleter>
typename prstorage::SimpleStorage<Element, Marshaller, Deleter>::element
prstorage::SimpleStorage<Element, Marshaller, Deleter>::find(
    std::function<bool(const SimpleStorage::element&)> is) const
{
  auto it = std::find_if(
      mElements.begin(dbstl::ReadModifyWriteOption::no_read_modify_write(),
                      true),
      mElements.end(), is);

  if (mElements.cend() != it) {
    return *it;
  }

  throw std::range_error("not found element");
}

template <typename Element, typename Marshaller, typename Deleter>
Deleter& prstorage::SimpleStorage<Element, Marshaller, Deleter>::getDeleter()
{
  return mDeleter;
}

template <typename Element, typename Marshaller, typename Deleter>
std::vector<
    typename prstorage::SimpleStorage<Element, Marshaller, Deleter>::element>
prstorage::SimpleStorage<Element, Marshaller, Deleter>::get_if(
    std::function<bool(const element&)> p) const
{
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

#endif  // SIMPLESTORAGE_H
