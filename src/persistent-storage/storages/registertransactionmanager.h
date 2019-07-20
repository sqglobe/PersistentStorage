#ifndef REGISTERTRANSACTIONMANAGER_H
#define REGISTERTRANSACTIONMANAGER_H

#include <db_cxx.h>

namespace prstorage {
class RegisterTransactionManager {
 public:
  /**
   * @brief Конструктор класса, начинает транзакцию
   */
  RegisterTransactionManager(DbEnv* env);
  ~RegisterTransactionManager();
  void commit();
  void abort();

 private:
  DbEnv* mEnv;
  DbTxn* mTxn;
};
}  // namespace prstorage

#endif  // REGISTERTRANSACTIONMANAGER_H
