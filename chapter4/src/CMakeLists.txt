SET(SRC_LIST main.cpp)
MESSAGE(STAUTS  "this is a binary dir: ", ${CHAPTER4_BINARY_DIR})
MESSAGE(STAUTS  "this is a source dir: ", ${CHAPTER4_SOURCE_DIR})
ADD_EXECUTABLE(chapter4 ${SRC_LIST})

