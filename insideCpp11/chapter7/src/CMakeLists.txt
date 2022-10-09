SET(SRC_LIST main.cpp)
MESSAGE(STAUTS  "this is a binary dir: ", ${CHAPTER7_BINARY_DIR})
MESSAGE(STAUTS  "this is a source dir: ", ${CHAPTER7_SOURCE_DIR})
ADD_EXECUTABLE(chapter7 ${SRC_LIST})

