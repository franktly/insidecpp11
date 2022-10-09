SET(SRC_LIST main.cpp)
MESSAGE(STAUTS  "this is a binary dir: ", ${LIBRARY_BINARY_DIR})
MESSAGE(STAUTS  "this is a source dir: ", ${LIBRARY_SOURCE_DIR})
ADD_EXECUTABLE(library ${SRC_LIST})
TARGET_LINK_LIBRARIES(library pthread)

