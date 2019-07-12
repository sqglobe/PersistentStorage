# PersistentStorage
Проект предоставляет хранилище данных с записью на диск

## Сборка проекта
Для сборки может понадобится указать флаги:  

* `BerkeleyDB_ROOT_DIR`
* `THREADS_PREFER_PTHREAD_FLAG` **ON**

Сборка и установка проекта выполняется командами:

```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=../installed -DBerkeleyDB_ROOT_DIR=/usr/lib ..
make install
```

Конфигурация по сборке библиотеки взята из репозитория - https://github.com/pablospe/cmake-example-library