#include <QtTest>
#include "persistent-storage/storages/storage.h"
#include "persistent-storage/storages/childstorage.h"
#include "persistent-storage/utils/store_primitives.h"

#include "persistent-storage/deleters/defaultchilddeleter.h"
#include "persistent-storage/deleters/parentsdeleter.h"
#include "persistent-storage/deleters/childthatisparentdeleter.h"

#include "persistent-storage/watchers/eventqueuewatcher.h"

struct TestElement{
    std::string id;
    std::string name;
};

std::string get_id(const TestElement &elem){
    return elem.id;
}



class TestMarshaller {
   public:
    static void restore(TestElement& elem, const void* src){
        src = restore_str(elem.id, src);
        src = restore_str(elem.name, src);
    }
    static u_int32_t size(const TestElement& element){
        u_int32_t size = 0;
        size += sizeof(std::string::size_type) + element.id.length();
        size += sizeof(std::string::size_type) + element.name.length();
        return size;

    }
    static void store(void* dest, const TestElement& elem){
        dest = save_str(elem.id, dest);
        dest = save_str(elem.name, dest);
    }
};

int get_dest_secdb_callback(Db * /* secondary */, const Dbt * /* key */,
    const Dbt *data, Dbt *result)
{
    TestElement el;
    TestMarshaller::restore(el, data->get_data());

    if(auto chars = static_cast<char *>(malloc(el.name.size() + 1))){
       result->set_flags(DB_DBT_APPMALLOC);
       strncpy(chars, el.name.c_str(), el.name.size() + 1);
       result->set_data(chars);
       result->set_size(static_cast<u_int32_t>(el.name.size()) + 1);
       return 0;
    }

    return 1;
}


class StoreWithWatcherTest : public QObject {
  Q_OBJECT

 public:
    StoreWithWatcherTest();

 public:
 private Q_SLOTS:
   void testAddWatcher();
   void testRemoveParent();
   void cleanup();
   void cleanupTestCase();

private:
   DbEnv* penv;
   Db* db;
   Db *secdb;
   Db *parent_db;
};




StoreWithWatcherTest::StoreWithWatcherTest()
{
    dbstl::dbstl_startup();
    penv = new DbEnv(DB_CXX_NO_EXCEPTIONS);
    auto res = penv->open(".", DB_INIT_LOCK | DB_INIT_LOG |  DB_INIT_MPOOL | DB_INIT_TXN | DB_CREATE | DB_PRIVATE | DB_RECOVER | DB_THREAD, 0600 );

    QCOMPARE(0, res);

    dbstl::register_db_env(penv);

    db = new Db(penv, DB_CXX_NO_EXCEPTIONS);
    res = db->open(nullptr, "StoreWithWatcherTest.db", "master", DB_BTREE, DB_CREATE | DB_THREAD | DB_AUTO_COMMIT, 0600 );

    QCOMPARE(0, res);

    dbstl::register_db(db);

    secdb = new Db(penv, DB_CXX_NO_EXCEPTIONS);

    secdb->set_flags(DB_DUP);
    res = secdb->open(nullptr, "StoreWithWatcherTest.db", "secondary" , DB_BTREE, DB_CREATE | DB_THREAD | DB_AUTO_COMMIT, 0600);

    QCOMPARE(0, res);
    dbstl::register_db(secdb);

    parent_db = new Db(penv, DB_CXX_NO_EXCEPTIONS);
    res = parent_db->open(nullptr, "StoreWithWatcherTest.db", "parent" , DB_BTREE, DB_CREATE | DB_THREAD | DB_AUTO_COMMIT, 0600);

    QCOMPARE(0, res);

    dbstl::register_db(parent_db);

    db->associate(nullptr, secdb, get_dest_secdb_callback, DB_CREATE);
}

void StoreWithWatcherTest::testAddWatcher()
{
    using ChildContainerType = ChildStorage<TestElement, TestElement, TestMarshaller, EventQueueWatcher<TestElement>>;
    using ParentDeleterType = ParentsDeleter<decltype(get_id(std::declval<TestElement>())), TestElement, ChildContainerType>;
    using ParentContainerType =  Storage<TestElement, TestMarshaller, EventQueueWatcher<TestElement>, DefaultTransactionManager, ParentDeleterType>;

    std::shared_ptr<ChildContainerType> child_container = std::make_shared<ChildContainerType>(db, secdb, penv);
    std::shared_ptr<ParentContainerType> parent_container = std::make_shared<ParentContainerType>(parent_db, penv, ParentDeleterType(child_container));

    auto countParentCalled = 0;
    auto parentAddHandler = [&countParentCalled](EnqueuedEvents event, const TestElement &elem){
        QCOMPARE(event, EnqueuedEvents::ADDED);
        QCOMPARE(elem.id, "parent id 1");
        countParentCalled++;
    };

    auto countChildCalled = 0;
    auto childAddHandler = [&countChildCalled](EnqueuedEvents event, const TestElement &elem){
        QCOMPARE(event, EnqueuedEvents::ADDED);
        QCOMPARE(elem.id, "child id 1");
        countChildCalled++;
    };

    parent_container->appendPermanentListener(EnqueuedEvents::ADDED, parentAddHandler);
    parent_container->appendPermanentListener(EnqueuedEvents::ALL_EVENTS, parentAddHandler);

    child_container->appendPermanentListener(EnqueuedEvents::ADDED, childAddHandler);
    child_container->appendPermanentListener(EnqueuedEvents::ADDED | EnqueuedEvents::UPDATED, childAddHandler);


    parent_container->add({"parent id 1", "parent name 1"});

    child_container->add({"child id 1", "parent id 1"});

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    QCOMPARE(countParentCalled, 2);
    QCOMPARE(countChildCalled, 2);
}

void StoreWithWatcherTest::testRemoveParent()
{
    using ChildContainerType = ChildStorage<TestElement, TestElement, TestMarshaller, EventQueueWatcher<TestElement>>;
    using ParentDeleterType = ParentsDeleter<decltype(get_id(std::declval<TestElement>())), TestElement, ChildContainerType>;
    using ParentContainerType =  Storage<TestElement, TestMarshaller, EventQueueWatcher<TestElement>, DefaultTransactionManager, ParentDeleterType>;

    std::shared_ptr<ChildContainerType> child_container = std::make_shared<ChildContainerType>(db, secdb, penv);
    std::shared_ptr<ParentContainerType> parent_container = std::make_shared<ParentContainerType>(parent_db, penv, ParentDeleterType(child_container));

    auto countParentCalled = 0;
    auto parentRemoveHandler = [&countParentCalled](EnqueuedEvents event, const TestElement &elem){
        QCOMPARE(event, EnqueuedEvents::DELETED);
        QCOMPARE(elem.id, "parent id 1");
        countParentCalled++;
    };

    auto countChildCalled = 0;
    auto childRemoveHandler = [&countChildCalled](EnqueuedEvents event, const TestElement &elem){
        QCOMPARE(event, EnqueuedEvents::DELETED);
        QCOMPARE(elem.id, "child id 1");
        countChildCalled++;
    };

    parent_container->appendPermanentListener(EnqueuedEvents::DELETED, parentRemoveHandler);
    auto parentHolder = parent_container->appendListener(EnqueuedEvents::UPDATED | EnqueuedEvents::DELETED, parentRemoveHandler);

    child_container->appendPermanentListener(EnqueuedEvents::DELETED, childRemoveHandler);
    auto childListener = child_container->appendListener(EnqueuedEvents::DELETED | EnqueuedEvents::UPDATED, childRemoveHandler);


    parent_container->add({"parent id 1", "parent name 1"});

    child_container->add({"child id 1", "parent id 1"});


    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    QVERIFY(parent_container->remove("parent id 1"));

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    QCOMPARE(countParentCalled, 2);
    QCOMPARE(countChildCalled, 2);
}


void StoreWithWatcherTest::cleanup()
{
    parent_db->truncate(nullptr, nullptr, 0);
    db->truncate(nullptr, nullptr, 0);
}

void StoreWithWatcherTest::cleanupTestCase()
{
    dbstl::dbstl_exit();
    if(penv)
        delete penv;
}

QTEST_APPLESS_MAIN(StoreWithWatcherTest)

#include "storewithwatchertest.moc"
