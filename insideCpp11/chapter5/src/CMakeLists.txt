SET(SRC_LIST main.cpp)
MESSAGE(STAUTS  "this is a binary dir: ", ${CHAPTER5_BINARY_DIR})
MESSAGE(STAUTS  "this is a source dir: ", ${CHAPTER5_SOURCE_DIR})
ADD_EXECUTABLE(chapter5 ${SRC_LIST})

