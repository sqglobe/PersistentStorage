#include "defaulttransactionmanager.h"
#include <dbstl_common.h>

DefaultTransactionManager::DefaultTransactionManager(DbEnv *env): mEnv(env), mTxn(env ?  dbstl::begin_txn(DB_TXN_SYNC| DB_TXN_WAIT, env) : nullptr)
{
}

DefaultTransactionManager::~DefaultTransactionManager()
{
    if( mEnv && mTxn ){
       dbstl::abort_txn(mEnv, mTxn);
    }
}

void DefaultTransactionManager::commit()
{

    if(mEnv && mTxn){
        dbstl::commit_txn(mEnv, mTxn);
        mEnv = nullptr;
        mTxn = nullptr;
    }
}

void DefaultTransactionManager::abort()
{
    if(mEnv && mTxn){
        dbstl::abort_txn(mEnv, mTxn);
        mEnv = nullptr;
        mTxn = nullptr;
    }

}
