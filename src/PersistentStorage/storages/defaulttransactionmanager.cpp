#include "defaulttransactionmanager.h"
#include <dbstl_common.h>


DefaultTransactionManager::DefaultTransactionManager(Db *db): mDb(db), mTxn(nullptr)
{
    if(mDb){
        mTxn = dbstl::begin_txn(DB_TXN_SYNC| DB_TXN_WAIT, db->get_env());
    }
}

DefaultTransactionManager::~DefaultTransactionManager()
{
    if( mDb && mTxn ){
       dbstl::abort_txn(mDb->get_env(), mTxn);
    }
}

void DefaultTransactionManager::commit()
{
    if(mDb && mTxn){
        dbstl::commit_txn(mDb->get_env(), mTxn);
        mDb = nullptr;
        mTxn = nullptr;
    }
}

void DefaultTransactionManager::abort()
{
    if(mDb && mTxn){
        dbstl::abort_txn(mDb->get_env(), mTxn);
        mDb = nullptr;
        mTxn = nullptr;
    }

}
