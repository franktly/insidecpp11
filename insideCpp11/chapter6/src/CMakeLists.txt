SET(SRC_LIST main.cpp)
MESSAGE(STAUTS  "this is a binary dir: ", ${CHAPTER6_BINARY_DIR})
MESSAGE(STAUTS  "this is a source dir: ", ${CHAPTER6_SOURCE_DIR})
ADD_EXECUTABLE(chapter6 ${SRC_LIST})
TARGET_LINK_LIBRARIES(chapter6 pthread)

