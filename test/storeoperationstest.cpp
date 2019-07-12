#include <QtTest>
#include "persistent-storage/storages/storage.h"
#include "persistent-storage/utils/store_primitives.h"



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


class StoreOperationsTest : public QObject {
  Q_OBJECT

 public:
 private Q_SLOTS:
   void testStoreInsertAndFetch();
   void testRemoveOperation();
   void testUpdateOperation();
   void testStrictUpdate();
   void testElementsAccess();
   void testWrapper();
};



void StoreOperationsTest::testStoreInsertAndFetch()
{
   Storage<TestElement, TestMarshaller, TestWatcher> store;
   TestElement elem1{"test id 1", "test name 1"}, elem2{"test id 2", "test name 2"};
   QVERIFY(store.add(elem1));
   QVERIFY(store.add(elem2));

   auto retrieve1 = store.get("test id 1");

   QCOMPARE(elem1.id, retrieve1.id);
   QCOMPARE(elem1.name, retrieve1.name);

}

void StoreOperationsTest::testRemoveOperation()
{
    Storage<TestElement, TestMarshaller, TestWatcher> store;
    TestElement elem1{"test id 1", "test name 1"}, elem2{"test id 2", "test name 2"};
    QVERIFY(store.add(elem1));
    QVERIFY(store.add(elem2));

    QVERIFY(store.remove("test id 2"));
    QVERIFY(!store.remove("test id 2"));

    QVERIFY_EXCEPTION_THROWN(store.get("test id 2"), std::range_error);

    auto retrieve1 = store.get("test id 1");

    QCOMPARE(elem1.id, retrieve1.id);
    QCOMPARE(elem1.name, retrieve1.name);
}

void StoreOperationsTest::testUpdateOperation()
{
    Storage<TestElement, TestMarshaller, TestWatcher> store;
    TestElement elem1{"test id 1", "test name 1"}, elem2{"test id 2", "test name 2"};
    QVERIFY(store.add(elem1));
    QVERIFY(store.add(elem2));
    TestElement elem3 {"test id 3", "test name 3"};


    store.update(elem3);

    QVERIFY(store.has("test id 3"));

    auto retrieve3 = store.get("test id 3");

    QCOMPARE(elem3.id, retrieve3.id);
    QCOMPARE(elem3.name, retrieve3.name);

    elem3.name = "new test 3";

    store.update(elem3);

    retrieve3 = store.get("test id 3");

    QCOMPARE(elem3.id, retrieve3.id);
    QCOMPARE(elem3.name, retrieve3.name);
}

void StoreOperationsTest::testStrictUpdate()
{
    Storage<TestElement, TestMarshaller, TestWatcher> store;
    TestElement elem1{"test id 1", "test name 1"}, elem2{"test id 2", "test name 2"};
    QVERIFY(store.add(elem1));
    QVERIFY(store.add(elem2));

    TestElement elem3 {"test id 3", "test name 3"};

    QVERIFY(!store.strictUpdate(elem3));

    elem2.name = "new name 2";


    QVERIFY(store.strictUpdate(elem2));

    auto retrieve2 = store.get("test id 2");

    QCOMPARE(elem2.id, retrieve2.id);
    QCOMPARE(elem2.name, retrieve2.name);
}

void StoreOperationsTest::testElementsAccess()
{
    Storage<TestElement, TestMarshaller, TestWatcher> store;
    TestElement elem1{"test id 1", "test name 1"}, elem2{"test id 2", "test name 2"}, elem3{"test id 3", "test name 3"};
    QVERIFY(store.add(elem1));
    QVERIFY(store.add(elem2));
    QVERIFY(store.add(elem3));

    QCOMPARE(3, store.size());

    auto elems = store.getAllElements();

    QCOMPARE(3, elems.size());


    auto get_if = store.get_if([](const TestElement &el){
        return el.name == "test name 1" || el.name == "test name 3";
    });

    QCOMPARE(get_if.size(), 2);

}

void StoreOperationsTest::testWrapper()
{
    std::shared_ptr<Storage<TestElement, TestMarshaller, TestWatcher, DefaultTransactionManager, DefaultDeleter<std::string, TestElement>>> store = std::make_shared<Storage<TestElement, TestMarshaller, TestWatcher, DefaultTransactionManager, DefaultDeleter<std::string, TestElement>>>();
    TestElement elem1{"test id 1", "test name 1"}, elem2{"test id 2", "test name 2"}, elem3{"test id 3", "test name 3"};
    QVERIFY(store->add(elem1));
    QVERIFY(store->add(elem2));
    QVERIFY(store->add(elem3));

    auto wrapper = store->wrapper("test id 1");
    wrapper->name = "new name 1";
    QVERIFY(wrapper.save());

    QCOMPARE(store->get("test id 1").name, "new name 1");

    wrapper.remove();

    QVERIFY(!store->has("test id 1"));

    auto wrapper2 = store->wrapper("test id 2");

    store->update({"test id 2", "new name 2"});

    QCOMPARE(wrapper2->name, "test name 2");

    wrapper2.reload();

    QCOMPARE(wrapper2->name, "new name 2");
}

QTEST_APPLESS_MAIN(StoreOperationsTest)

#include "storeoperationstest.moc"
