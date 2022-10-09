SET(SRC_LIST main.cpp)
MESSAGE(STAUTS  "this is a binary dir: ", ${LANGUAGE_BINARY_DIR})
MESSAGE(STAUTS  "this is a source dir: ", ${LANGUAGE_SOURCE_DIR})
ADD_EXECUTABLE(language ${SRC_LIST})
TARGET_LINK_LIBRARIES(language pthread)

