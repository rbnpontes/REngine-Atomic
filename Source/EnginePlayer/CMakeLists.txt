file (GLOB SOURCE_FILES *.cpp *.h)

add_library(${ENGINE_PLAYER_TARGET_LIB} ${SOURCE_FILES})
target_link_libraries(${ENGINE_PLAYER_TARGET_LIB} ${ENGINE_CORE_LIB_TARGET})
target_compile_definitions(${ENGINE_PLAYER_TARGET_LIB} PUBLIC 
    -DENGINE_PLAYER_TARGET="${ENGINE_PLAYER_TARGET}")
add_subdirectory(Application)

vs_add_to_grp(${ENGINE_PLAYER_TARGET_LIB} "${VS_GRP_ENGINE_SRC}")