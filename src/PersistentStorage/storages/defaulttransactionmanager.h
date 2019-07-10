#ifndef DEFAULTTRANSACTIONMANAGER_H
#define DEFAULTTRANSACTIONMANAGER_H

#include <db_cxx.h>

class DefaultTransactionManager{
public:
    DefaultTransactionManager(DbEnv *env);
    ~DefaultTransactionManager();
    void commit();
    void abort();
private:
    DbEnv *mEnv;
    DbTxn *mTxn;
};

#endif // DEFAULTTRANSACTIONMANAGER_H
