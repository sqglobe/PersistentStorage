project(PersistentStorageSource)

set(FILES_SOURCES
  stub.cpp
)

set(FILES_HEADERS
  PersistentStorage/storages/basecontainer.h

  PersistentStorage/wrappers/containerelementwrapper.h
  PersistentStorage/wrappers/transparentcontainerelementwrapper.h

)

add_library(PersistentStorage ${FILES_SOURCES} ${FILES_HEADERS})

set_target_properties(PersistentStorage PROPERTIES PUBLIC_HEADER "${FILES_HEADERS}")

