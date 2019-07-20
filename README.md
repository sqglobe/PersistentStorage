# persistent-storage
Проект предоставляет хранилище данных с записью на диск.

## Зависимости

Для сборки проекта понадобится:

* git
* cmake **3.8** и выше
* gcc 7 и выше
* [Berkeley DB](https://www.oracle.com/database/technologies/related/berkeleydb.html) версии **5.3** и выше
* QTest - для тестов

Проект использует [eventpp](https://github.com/wqking/eventpp) для работы с событиями.

## Сборка проекта
Для сборки может понадобится указать флаги:  

* `BerkeleyDB_ROOT_DIR`
* `THREADS_PREFER_PTHREAD_FLAG` **ON**

Перед сборкой, в папке с проектом необходимо выполнить:

```
git submodule init
git submodule update
```

Сборка и установка проекта выполняется командами:

```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=../installed -DBerkeleyDB_ROOT_DIR=/usr/lib ..
make install
```

Конфигурация по сборке библиотеки взята из репозитория - https://github.com/pablospe/cmake-example-library
