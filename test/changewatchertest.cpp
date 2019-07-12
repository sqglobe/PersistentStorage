#include <QtTest>
#include "persistent-storage/watchers/eventqueuewatcher.h"
#include "persistent-storage/watchers/enqueuedevents.h"


struct TestElement{
    std::string id;
    std::string name;
};

class TestElementWatcher: public EventQueueWatcher<TestElement>{
public:
    using super = EventQueueWatcher<TestElement>;
public:
    void elementAdded(const TestElement &element){
        super::elementAdded(element);
    }
    void elementRemoved(const TestElement &element){
        super::elementRemoved(element);
    }
    void elementUpdated(const TestElement &element){
        super::elementUpdated(element);
    }
};

class ChangeWatcherTest : public QObject {
  Q_OBJECT

 public:
 private Q_SLOTS:
   void testAddEventHandling();
   void testAddEventTemporaryHandler();

   void testUpdateEventHandling();
   void testUpdateEventTemporaryHandler();

   void testDeleteEventHandling();
   void testDeleteEventTemporaryHandler();

   void testTemporaryAndPermanentListeners();

};





void ChangeWatcherTest::testAddEventHandling()
{
    TestElementWatcher watcher;
    TestElement testEl {"added id", "added name"};
    auto calledCount = 0;

    auto addedListener = [testEl, &calledCount](EnqueuedEvents event, const TestElement &element){
        QCOMPARE(event, EnqueuedEvents::ADDED);
        QCOMPARE(testEl.id, element.id);
        QCOMPARE(testEl.name, element.name);
        calledCount++;
     };

    auto failedListener = [](EnqueuedEvents, const TestElement &){
        QVERIFY(false);
    };

    watcher.appendPermanentListener(EnqueuedEvents::ADDED, addedListener);

    watcher.appendPermanentListener(EnqueuedEvents::UPDATED, failedListener);

    watcher.appendPermanentListener(EnqueuedEvents::DELETED, failedListener);

    watcher.appendPermanentListener(EnqueuedEvents::ADDED | EnqueuedEvents::UPDATED, addedListener);

    watcher.appendPermanentListener(EnqueuedEvents::ALL_EVENTS, addedListener);

    watcher.appendPermanentListener(EnqueuedEvents::DELETED | EnqueuedEvents::UPDATED, failedListener);

    watcher.elementAdded(testEl);

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    QCOMPARE(calledCount, 3);

}

void ChangeWatcherTest::testAddEventTemporaryHandler()
{
    TestElementWatcher watcher;
    TestElement testEl {"added id", "added name"};
    auto calledCount = 0;

    auto addedListener = [testEl, &calledCount](EnqueuedEvents event, const TestElement &element){
        QCOMPARE(event, EnqueuedEvents::ADDED);
        QCOMPARE(testEl.id, element.id);
        QCOMPARE(testEl.name, element.name);
        calledCount++;
     };

    auto failedListener = [](EnqueuedEvents, const TestElement &){
        QVERIFY(false);
    };


    {
        auto addHandler = watcher.appendListener(EnqueuedEvents::ADDED, addedListener);
        auto updateHandler = watcher.appendListener(EnqueuedEvents::UPDATED, failedListener);
        auto deleteHandler = watcher.appendListener(EnqueuedEvents::DELETED, failedListener);
        auto allEventsHandler = watcher.appendListener(EnqueuedEvents::ALL_EVENTS, addedListener);
        auto add_deleteHandlers = watcher.appendListener(EnqueuedEvents::ADDED | EnqueuedEvents::DELETED, addedListener);
        watcher.elementAdded(testEl);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    watcher.elementAdded(testEl);

    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    QCOMPARE(calledCount, 3);

}

void ChangeWatcherTest::testUpdateEventHandling()
{
    TestElementWatcher watcher;
    TestElement testEl {"update id", "update name"};
    auto calledCount = 0;

    auto updatedListener = [testEl, &calledCount](EnqueuedEvents event, const TestElement &element){
        QCOMPARE(event, EnqueuedEvents::UPDATED);
        QCOMPARE(testEl.id, element.id);
        QCOMPARE(testEl.name, element.name);
        calledCount++;
     };

    auto failedListener = [](EnqueuedEvents, const TestElement &){
        QVERIFY(false);
    };

    watcher.appendPermanentListener(EnqueuedEvents::ADDED, failedListener);

    watcher.appendPermanentListener(EnqueuedEvents::UPDATED, updatedListener);

    watcher.appendPermanentListener(EnqueuedEvents::DELETED, failedListener);

    watcher.appendPermanentListener(EnqueuedEvents::ADDED | EnqueuedEvents::UPDATED, updatedListener);

    watcher.appendPermanentListener(EnqueuedEvents::ALL_EVENTS, updatedListener);

    watcher.appendPermanentListener(EnqueuedEvents::DELETED | EnqueuedEvents::ADDED, failedListener);

    watcher.elementUpdated(testEl);

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    QCOMPARE(calledCount, 3);
}

void ChangeWatcherTest::testUpdateEventTemporaryHandler()
{
    TestElementWatcher watcher;
    TestElement testEl {"updated id", "updated name"};
    auto calledCount = 0;

    auto updatedListener = [testEl, &calledCount](EnqueuedEvents event, const TestElement &element){
        QCOMPARE(event, EnqueuedEvents::UPDATED);
        QCOMPARE(testEl.id, element.id);
        QCOMPARE(testEl.name, element.name);
        calledCount++;
     };

    auto failedListener = [](EnqueuedEvents, const TestElement &){
        QVERIFY(false);
    };


    {
        auto addHandler = watcher.appendListener(EnqueuedEvents::ADDED, failedListener);
        auto updateHandler = watcher.appendListener(EnqueuedEvents::UPDATED, updatedListener);
        auto deleteHandler = watcher.appendListener(EnqueuedEvents::DELETED, failedListener);
        auto allEventsHandler = watcher.appendListener(EnqueuedEvents::ALL_EVENTS, updatedListener);
        auto add_deleteHandlers = watcher.appendListener(EnqueuedEvents::UPDATED | EnqueuedEvents::DELETED, updatedListener);
        watcher.elementUpdated(testEl);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    watcher.elementUpdated(testEl);

    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    QCOMPARE(calledCount, 3);
}

void ChangeWatcherTest::testDeleteEventHandling()
{
    TestElementWatcher watcher;
    TestElement testEl {"delete id", "delete name"};
    auto calledCount = 0;

    auto deletedListener = [testEl, &calledCount](EnqueuedEvents event, const TestElement &element){
        QCOMPARE(event, EnqueuedEvents::DELETED);
        QCOMPARE(testEl.id, element.id);
        QCOMPARE(testEl.name, element.name);
        calledCount++;
     };

    auto failedListener = [](EnqueuedEvents, const TestElement &){
        QVERIFY(false);
    };

    watcher.appendPermanentListener(EnqueuedEvents::ADDED, failedListener);

    watcher.appendPermanentListener(EnqueuedEvents::UPDATED, failedListener);

    watcher.appendPermanentListener(EnqueuedEvents::DELETED, deletedListener);

    watcher.appendPermanentListener(EnqueuedEvents::ADDED | EnqueuedEvents::UPDATED, failedListener);

    watcher.appendPermanentListener(EnqueuedEvents::ALL_EVENTS, deletedListener);

    watcher.appendPermanentListener(EnqueuedEvents::DELETED | EnqueuedEvents::ADDED, deletedListener);

    watcher.elementRemoved(testEl);

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    QCOMPARE(calledCount, 3);
}

void ChangeWatcherTest::testDeleteEventTemporaryHandler()
{
    TestElementWatcher watcher;
    TestElement testEl {"deleted id", "deleted name"};
    auto calledCount = 0;

    auto deletedListener = [testEl, &calledCount](EnqueuedEvents event, const TestElement &element){
        QCOMPARE(event, EnqueuedEvents::DELETED);
        QCOMPARE(testEl.id, element.id);
        QCOMPARE(testEl.name, element.name);
        calledCount++;
     };

    auto failedListener = [](EnqueuedEvents, const TestElement &){
        QVERIFY(false);
    };


    {
        auto addHandler = watcher.appendListener(EnqueuedEvents::ADDED, failedListener);
        auto updateHandler = watcher.appendListener(EnqueuedEvents::UPDATED, failedListener);
        auto deleteHandler = watcher.appendListener(EnqueuedEvents::DELETED, deletedListener);
        auto allEventsHandler = watcher.appendListener(EnqueuedEvents::ALL_EVENTS, deletedListener);
        auto add_deleteHandlers = watcher.appendListener(EnqueuedEvents::UPDATED | EnqueuedEvents::DELETED, deletedListener);
        watcher.elementRemoved(testEl);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    watcher.elementRemoved(testEl);

    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    QCOMPARE(calledCount, 3);
}

void ChangeWatcherTest::testTemporaryAndPermanentListeners()
{
    TestElementWatcher watcher;

    auto addListenerCalledCount = 0;
    auto addListener = [&addListenerCalledCount](EnqueuedEvents event, const TestElement &){
        QCOMPARE(event, EnqueuedEvents::ADDED);
        addListenerCalledCount++;
    };

    auto updateListenerCalledCount = 0;
    auto updateListener = [&updateListenerCalledCount](EnqueuedEvents event, const TestElement &){
        QCOMPARE(event, EnqueuedEvents::UPDATED);
        updateListenerCalledCount++;
    };

    auto deleteListenerCalledCount = 0;
    auto deleteListener = [&deleteListenerCalledCount](EnqueuedEvents event, const TestElement &){
        QCOMPARE(event, EnqueuedEvents::DELETED);
        deleteListenerCalledCount++;
    };

    auto delete_and_update_ListenerCalledCount = 0;
    auto delete_and_updateListener = [&delete_and_update_ListenerCalledCount](EnqueuedEvents event, const TestElement &){
        QVERIFY(EnqueuedEvents::DELETED == event || EnqueuedEvents::UPDATED == event);
        delete_and_update_ListenerCalledCount++;
    };

    auto update_and_add_ListenerCalledCount = 0;
    auto update_and_addListener = [&update_and_add_ListenerCalledCount](EnqueuedEvents event, const TestElement &){
        QVERIFY(EnqueuedEvents::UPDATED == event || EnqueuedEvents::ADDED == event);
        update_and_add_ListenerCalledCount++;
    };

    auto allEvent_ListenerCalledCount = 0;
    auto allEvent_addListener = [&allEvent_ListenerCalledCount](EnqueuedEvents, const TestElement &){
        allEvent_ListenerCalledCount++;
    };

    watcher.appendPermanentListener(EnqueuedEvents::ADDED, addListener);
    watcher.appendPermanentListener(EnqueuedEvents::UPDATED, updateListener);
    watcher.appendPermanentListener(EnqueuedEvents::DELETED, deleteListener);
    watcher.appendPermanentListener(EnqueuedEvents::DELETED | EnqueuedEvents::UPDATED, delete_and_updateListener);
    watcher.appendPermanentListener(EnqueuedEvents::UPDATED | EnqueuedEvents::ADDED, update_and_addListener);
    watcher.appendPermanentListener(EnqueuedEvents::ALL_EVENTS, allEvent_addListener);

    watcher.elementAdded({"Test id 1", "Test name 1"});
    watcher.elementUpdated({"Test id 2", "Test name 2"});
    watcher.elementRemoved({"Test id 3", "Test name 3"});
    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    {
        auto addedHandler = watcher.appendListener(EnqueuedEvents::ADDED, addListener);
        auto newAddedHandler = std::move(addedHandler);
        auto updatedHandler = watcher.appendListener(EnqueuedEvents::UPDATED, updateListener);
        auto deletedHandler = watcher.appendListener(EnqueuedEvents::DELETED, deleteListener);
        auto updated_and_deletedHandler = watcher.appendListener(EnqueuedEvents::DELETED | EnqueuedEvents::UPDATED, delete_and_updateListener);
        auto updated_and_addedHandler = watcher.appendListener(EnqueuedEvents::UPDATED | EnqueuedEvents::ADDED, update_and_addListener);
        auto all_events_Handler = watcher.appendListener(EnqueuedEvents::ALL_EVENTS, allEvent_addListener);

        watcher.elementAdded({"Test id 4", "Test name 4"});
        watcher.elementUpdated({"Test id 5", "Test name 5"});
        watcher.elementRemoved({"Test id 6", "Test name 6"});
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    watcher.elementAdded({"Test id 7", "Test name 7"});
    watcher.elementUpdated({"Test id 8", "Test name 8"});
    watcher.elementRemoved({"Test id 9", "Test name 9"});

    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    QCOMPARE(addListenerCalledCount, 4);
    QCOMPARE(updateListenerCalledCount, 4);
    QCOMPARE(deleteListenerCalledCount, 4);

    QCOMPARE(delete_and_update_ListenerCalledCount, 8);
    QCOMPARE(update_and_add_ListenerCalledCount, 8);
    QCOMPARE(allEvent_ListenerCalledCount, 12);
}

QTEST_APPLESS_MAIN(ChangeWatcherTest)

#include "changewatchertest.moc"
