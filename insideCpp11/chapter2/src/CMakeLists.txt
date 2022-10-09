SET(SRC_LIST main.cpp)
MESSAGE(STAUTS  "this is a binary dir: ", ${CHAPTER2_BINARY_DIR})
MESSAGE(STAUTS  "this is a source dir: ", ${CHAPTER2_SOURCE_DIR})
ADD_EXECUTABLE(chapter2 ${SRC_LIST})

