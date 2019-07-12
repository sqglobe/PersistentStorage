#include <QtTest>
#include "PersistentStorage/storages/storage.h"
#include "PersistentStorage/storages/childstorage.h"
#include "PersistentStorage/utils/store_primitives.h"

#include "PersistentStorage/deleters/defaultchilddeleter.h"
#include "PersistentStorage/deleters/parentsdeleter.h"
#include "PersistentStorage/deleters/childthatisparentdeleter.h"


struct TestElement{
    std::string id;
    std::string name;
};

std::string get_id(const TestElement &elem){
    return elem.id;
}


class TestWatcher{
protected:
    void elementAdded(const TestElement &) {}
    void elementRemoved(const TestElement &) {}
    void elementUpdated(const TestElement &) {}
};

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


class ChildStorageTest : public QObject {
  Q_OBJECT

 public:
    ChildStorageTest();

 public:
 private Q_SLOTS:
   void testStorageCreation();
   void testSeveralChilds();
   void testSeveralLevelsOfInheritance();
   void testWrapperInChildContainer();
   void cleanup();
   void cleanupTestCase();

private:
   DbEnv* penv;
   Db* db;
   Db *secdb;
   Db *parent_db;
};




ChildStorageTest::ChildStorageTest()
{
    dbstl::dbstl_startup();
    penv = new DbEnv(DB_CXX_NO_EXCEPTIONS);
    auto res = penv->open(".", DB_INIT_LOCK |  DB_INIT_MPOOL | DB_INIT_TXN | DB_CREATE | DB_PRIVATE | DB_RECOVER | DB_THREAD, 0600 );

    QCOMPARE(0, res);

    dbstl::register_db_env(penv);

    db = new Db(penv, DB_CXX_NO_EXCEPTIONS);
    res = db->open(nullptr, "ChildStorageTest_testStorageCreation.db", "master", DB_BTREE, DB_CREATE | DB_THREAD | DB_AUTO_COMMIT, 0600 );

    QCOMPARE(0, res);

    dbstl::register_db(db);

    secdb = new Db(penv, DB_CXX_NO_EXCEPTIONS);

    secdb->set_flags(DB_DUP);
    res = secdb->open(nullptr, "ChildStorageTest_testStorageCreation.db", "secondary" , DB_BTREE, DB_CREATE | DB_THREAD | DB_AUTO_COMMIT, 0600);

    QCOMPARE(0, res);
    dbstl::register_db(secdb);

    parent_db = new Db(penv, DB_CXX_NO_EXCEPTIONS);
    res = parent_db->open(nullptr, "ChildStorageTest_testStorageCreation.db", "parent" , DB_BTREE, DB_CREATE | DB_THREAD | DB_AUTO_COMMIT, 0600);

    QCOMPARE(0, res);

    dbstl::register_db(parent_db);

    db->associate(nullptr, secdb, get_dest_secdb_callback, DB_CREATE);
}

void ChildStorageTest::testStorageCreation()
{
   using ChildContainerType = ChildStorage<TestElement, TestElement, TestMarshaller, TestWatcher>;
   using ParentDeleterType = ParentsDeleter<decltype(get_id(std::declval<TestElement>())), TestElement, ChildContainerType>;
   using ParentContainerType =  Storage<TestElement, TestMarshaller, TestWatcher, DefaultTransactionManager, ParentDeleterType>;

   std::shared_ptr<ChildContainerType> child_container = std::make_shared<ChildContainerType>(db, secdb, penv);
   std::shared_ptr<ParentContainerType> parent_container = std::make_shared<ParentContainerType>(parent_db, penv, ParentDeleterType(child_container));

   parent_container->add({"parent id 1", "parent name 1"});
   parent_container->add({"parent id 2", "parent name 2"});

   child_container->add({"child id 1", "parent id 1"});
   child_container->add({"child id 2", "parent id 2"});

   QVERIFY(parent_container->remove("parent id 1"));

   QVERIFY(!child_container->has("child id 1"));
   QVERIFY(child_container->has("child id 2"));
}

void ChildStorageTest::testSeveralChilds()
{
    using ChildContainerType = ChildStorage<TestElement, TestElement, TestMarshaller, TestWatcher>;
    using ParentDeleterType = ParentsDeleter<decltype(get_id(std::declval<TestElement>())), TestElement, ChildContainerType>;
    using ParentContainerType =  Storage<TestElement, TestMarshaller, TestWatcher, DefaultTransactionManager, ParentDeleterType>;

    std::shared_ptr<ChildContainerType> child_container = std::make_shared<ChildContainerType>(db, secdb, penv);
    std::shared_ptr<ParentContainerType> parent_container = std::make_shared<ParentContainerType>(parent_db, penv, ParentDeleterType(child_container));

    parent_container->add({"parent id 1", "parent name 1"});
    parent_container->add({"parent id 2", "parent name 2"});

    child_container->add({"child id 1", "parent id 1"});
    child_container->add({"child id 1_2", "parent id 1"});
    child_container->add({"child id 2", "parent id 2"});

    QVERIFY(parent_container->remove("parent id 1"));

    QVERIFY(!child_container->has("child id 1"));
    QVERIFY(!child_container->has("child id 1_2"));
    QVERIFY(child_container->has("child id 2"));

}

void ChildStorageTest::testSeveralLevelsOfInheritance()
{
    using ChildContainerType = ChildStorage<TestElement, TestElement, TestMarshaller, TestWatcher>;
    using KeyType = decltype(get_id(std::declval<TestElement>()));
    using ChildThatIsParentDeleterType = ChildThatIsParentDeleter<KeyType, TestElement, TestElement, ChildContainerType> ;
    using ChildThatIsParentContainerType = ChildStorage<TestElement, TestElement, TestMarshaller, TestWatcher, DefaultTransactionManager, ChildThatIsParentDeleterType>;
    using ParentDeleterType = ParentsDeleter<KeyType, TestElement, ChildThatIsParentContainerType>;
    using ParentContainerType = Storage<TestElement, TestMarshaller, TestWatcher, DefaultTransactionManager, ParentDeleterType>;


    auto child_child_db =  new Db(penv, DB_CXX_NO_EXCEPTIONS);;
    auto res = child_child_db->open(nullptr, "ChildStorageTest_testStorageCreation.db", "child_child" , DB_BTREE, DB_CREATE | DB_THREAD | DB_AUTO_COMMIT, 0600);
    dbstl::register_db(child_child_db);
    QCOMPARE(0, res);

    auto  child_child_secdb = new Db(penv, DB_CXX_NO_EXCEPTIONS);

    dbstl::register_db(child_child_secdb);

    child_child_secdb->set_flags(DB_DUP);
    res = child_child_secdb->open(nullptr, "ChildStorageTest_testStorageCreation.db", "child_child_secdb" , DB_BTREE, DB_CREATE | DB_THREAD | DB_AUTO_COMMIT, 0600);

    QCOMPARE(0, res);

    child_child_db->associate(nullptr, child_child_secdb, get_dest_secdb_callback, DB_CREATE);

    child_child_db->truncate(nullptr, nullptr, 0);

    std::shared_ptr<ChildContainerType> child_container = std::make_shared<ChildContainerType>(db, secdb, penv);
    std::shared_ptr<ChildThatIsParentContainerType> child_that_is_parent =
            std::make_shared<ChildThatIsParentContainerType>(child_child_db, child_child_secdb, penv, ChildThatIsParentDeleterType(child_container) );
    std::shared_ptr<ParentContainerType> parent_container = std::make_shared<ParentContainerType>(parent_db, penv, ParentDeleterType(child_that_is_parent));

    parent_container->add({"parent id 1", "parent name 1"});
    parent_container->add({"parent id 2", "parent name 2"});


    child_that_is_parent->add({"child parent id 1", "parent id 1"});
    child_that_is_parent->add({"child parent id 1_2", "parent id 1"});
    child_that_is_parent->add({"child parent id 2", "parent id 2"});

    child_container->add({"child id 1", "child parent id 1"});
    child_container->add({"child id 1_2", "child parent id 1"});
    child_container->add({"child id 1_2_1", "child parent id 1_2"});
    child_container->add({"child id 2", "child parent id 2"});

    QVERIFY(parent_container->remove("parent id 1"));

    QVERIFY(!child_that_is_parent->has("child parent id 1"));
    QVERIFY(!child_that_is_parent->has("child parent id 1_2"));
    QVERIFY(child_that_is_parent->has("child parent id 2"));

    QVERIFY(!child_container->has("child id 1"));
    QVERIFY(!child_container->has("child id 1_2"));
    QVERIFY(!child_container->has("child id 1_2_1"));

    QVERIFY(child_container->has("child id 2"));

}

void ChildStorageTest::testWrapperInChildContainer()
{
    using ChildContainerType = ChildStorage<TestElement, TestElement, TestMarshaller, TestWatcher>;
    using ParentDeleterType = ParentsDeleter<decltype(get_id(std::declval<TestElement>())), TestElement, ChildContainerType>;
    using ParentContainerType =  Storage<TestElement, TestMarshaller, TestWatcher, DefaultTransactionManager, ParentDeleterType>;

    std::shared_ptr<ChildContainerType> child_container = std::make_shared<ChildContainerType>(db, secdb, penv);
    std::shared_ptr<ParentContainerType> parent_container = std::make_shared<ParentContainerType>(parent_db, penv, ParentDeleterType(child_container));

    parent_container->add({"parent id 1", "parent name 1"});
    parent_container->add({"parent id 2", "parent name 2"});

    child_container->add({"child id 1", "parent id 1"});
    child_container->add({"child id 2", "parent id 2"});

    auto wrapper = child_container->wrapper("child id 1");

    wrapper->name = "test";

    QVERIFY(wrapper.save());

    QCOMPARE(child_container->get("child id 1").name, "test");
}

void ChildStorageTest::cleanup()
{
    parent_db->truncate(nullptr, nullptr, 0);
    db->truncate(nullptr, nullptr, 0);
}

void ChildStorageTest::cleanupTestCase()
{
    dbstl::dbstl_exit();
    if(penv)
        delete penv;
}

QTEST_APPLESS_MAIN(ChildStorageTest)

#include "childstoragetest.moc"
