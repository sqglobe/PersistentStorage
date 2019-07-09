#ifndef DEFAULTTRANSACTIONMANAGER_H
#define DEFAULTTRANSACTIONMANAGER_H

#include <db_cxx.h>

class DefaultTransactionManager{
public:
    DefaultTransactionManager(Db *db);
    ~DefaultTransactionManager();
    void commit();
    void abort();
private:
    Db *mDb;
    DbTxn *mTxn;
};

#endif // DEFAULTTRANSACTIONMANAGER_H
