#ifndef DEFAULTTRANSACTIONMANAGER_H
#define DEFAULTTRANSACTIONMANAGER_H

#include <db_cxx.h>

namespace prstorage {
/**
 * Менеджер транзакций. Начинает и завершает транзакцию
 */
class DefaultTransactionManager {
 public:
  /**
   * @brief Конструктор класса, начинает транзакцию
   */
  DefaultTransactionManager(DbEnv* env);
  ~DefaultTransactionManager();
  void commit();
  void abort();

 private:
  DbEnv* mEnv;
  DbTxn* mTxn;
};
}  // namespace prstorage

#endif  // DEFAULTTRANSACTIONMANAGER_H
