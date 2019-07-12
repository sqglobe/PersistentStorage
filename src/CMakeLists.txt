find_package(BerkeleyDB REQUIRED)
find_package (Threads REQUIRED)

set(FILES_SOURCES
  persistent-storage/utils/store_primitives.cpp
  persistent-storage/storages/defaulttransactionmanager.cpp
)

set(FILES_HEADERS
  persistent-storage/storages/storage.h
  persistent-storage/storages/childstorage.h
  persistent-storage/storages/defaulttransactionmanager.h

  persistent-storage/deleters/defaultdeleter.h
  persistent-storage/deleters/parentsdeleter.h
  persistent-storage/deleters/defaultchilddeleter.h
  persistent-storage/deleters/childthatisparentdeleter.h

  persistent-storage/wrappers/containerelementwrapper.h
  persistent-storage/wrappers/transparentcontainerelementwrapper.h

  persistent-storage/watchers/eventqueuewatcher.h
  persistent-storage/watchers/enqueuedevents.h
  persistent-storage/watchers/eventlistenerholder.h


  persistent-storage/utils/store_primitives.h
)

add_library(${LIBRARY_NAME} ${FILES_SOURCES} ${FILES_HEADERS})

target_link_libraries(${LIBRARY_NAME} PUBLIC ${CMAKE_THREAD_LIBS_INIT} Oracle::BerkeleyDB)

target_include_directories(${LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/submodules/eventpp/include)

include(${CMAKE_SOURCE_DIR}/cmake/LibraryConfig.cmake)
