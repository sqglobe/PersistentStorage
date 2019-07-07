project(PersistentStorageSource)

find_package(BerkeleyDB REQUIRED)


set(FILES_SOURCES
  PersistentStorage/utils/store_primitives.cpp
)

set(FILES_HEADERS
  PersistentStorage/storages/store.h

  PersistentStorage/wrappers/containerelementwrapper.h
  PersistentStorage/wrappers/transparentcontainerelementwrapper.h

  PersistentStorage/utils/store_primitives.h

)

add_library(PersistentStorage ${FILES_SOURCES} ${FILES_HEADERS})

target_link_libraries(PersistentStorage PRIVATE Oracle::BerkeleyDB)


set_target_properties(PersistentStorage PROPERTIES PUBLIC_HEADER "${FILES_HEADERS}")

