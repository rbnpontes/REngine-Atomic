file (GLOB SOURCE_FILES *.cpp *.h)

add_library(AtomicPlayerLib ${SOURCE_FILES})
target_link_libraries(AtomicPlayerLib ${ENGINE_CORE_LIB_TARGET})
add_subdirectory(Application)
