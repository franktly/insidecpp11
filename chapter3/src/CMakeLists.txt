SET(SRC_LIST main.cpp)
MESSAGE(STAUTS  "this is a binary dir: ", ${CHAPTER3_BINARY_DIR})
MESSAGE(STAUTS  "this is a source dir: ", ${CHAPTER3_SOURCE_DIR})
ADD_EXECUTABLE(chapter3 ${SRC_LIST})

