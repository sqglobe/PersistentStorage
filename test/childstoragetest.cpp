#include <QtTest>
#include "PersistentStorage/storages/storage.h"
#include "PersistentStorage/utils/store_primitives.h"



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


class ChildStorageTest : public QObject {
  Q_OBJECT

 public:
 private Q_SLOTS:
   void testStorageCreation();
};




void ChildStorageTest::testStorageCreation()
{

}

QTEST_APPLESS_MAIN(ChildStorageTest)

#include "childstoragetest.moc"
