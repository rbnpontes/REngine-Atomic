file (GLOB SOURCE_FILES *.cpp *.h)

add_library(AtomicPlayerLib ${SOURCE_FILES})
target_link_libraries(AtomicPlayerLib ${ENGINE_CORE_LIB_TARGET})
add_subdirectory(Application)

vs_add_to_grp(AtomicPlayerLib "${VS_GRP_ENGINE_SRC}")