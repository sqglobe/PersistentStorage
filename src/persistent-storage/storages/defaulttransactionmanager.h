#ifndef DEFAULTTRANSACTIONMANAGER_H
#define DEFAULTTRANSACTIONMANAGER_H

#include <db_cxx.h>

namespace prstorage {
class DefaultTransactionManager {
 public:
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
