#include "registertransactionmanager.h"

#include <dbstl_common.h>

using namespace prstorage;

RegisterTransactionManager::RegisterTransactionManager(DbEnv* env) :
    mEnv(env), mTxn(nullptr)
{
  if (env) {
    dbstl::register_db_env(env);
    mTxn = dbstl::begin_txn(DB_TXN_SYNC | DB_TXN_WAIT, env);
  }
}

RegisterTransactionManager::~RegisterTransactionManager()
{
  if (mEnv && mTxn) {
    dbstl::abort_txn(mEnv, mTxn);
  }
}

void RegisterTransactionManager::commit()
{
  if (mEnv && mTxn) {
    dbstl::commit_txn(mEnv, mTxn);
    mEnv = nullptr;
    mTxn = nullptr;
  }
}

void RegisterTransactionManager::abort()
{
  if (mEnv && mTxn) {
    dbstl::abort_txn(mEnv, mTxn);
    mEnv = nullptr;
    mTxn = nullptr;
  }
}
