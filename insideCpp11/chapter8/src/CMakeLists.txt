SET(SRC_LIST main.cpp)
MESSAGE(STAUTS  "this is a binary dir: ", ${CHAPTER8_BINARY_DIR})
MESSAGE(STAUTS  "this is a source dir: ", ${CHAPTER8_SOURCE_DIR})
ADD_EXECUTABLE(chapter8 ${SRC_LIST})

