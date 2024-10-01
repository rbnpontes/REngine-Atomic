set (BINDINGS_JS_BASE_PATH ${ENGINE_SOURCE_DIR}/Artifacts/Build/Source/Generated/JS)
set (ENGINE_CORE_BINDING_SRC ${BINDINGS_JS_BASE_PATH}/EngineCore)
file (GLOB JAVASCRIPT_SOURCE_FILES JavaScript/*.h JavaScript/*.cpp)

macro(add_js_binding binding_target src_path)
    file(GLOB root_files ${src_path}/*.h ${src_path}/*.cpp)
    file(GLOB struct_files ${src_path}/Structs/*.h ${src_path}/Structs/*.cpp)
    file(GLOB enum_files ${src_path}/Enums/*.h ${src_path}/Enums/*.cpp)
    
    list(APPEND target_src_files ${root_files})
    list(APPEND target_src_files ${struct_files})
    list(APPEND target_src_files ${enum_files})
    list(APPEND target_src_files ${JAVASCRIPT_SOURCE_FILES})
    add_library(${binding_target} MODULE ${target_src_files})

    target_include_directories(${binding_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/JavaScript)
    target_link_libraries(${binding_target} PRIVATE ${ENGINE_CORE_LIB_TARGET} ${ENGINE_PLUGIN_JS_INTERFACE_TARGET})
    target_compile_definitions(${binding_target} PRIVATE -DENGINE_PLUGIN_JS_TARGET="${ENGINE_PLUGIN_JS_TARGET}")

    vs_add_to_grp(${binding_target} "${VS_GRP_BINDINGS}")

    source_group("Shared" FILES ${JAVASCRIPT_SOURCE_FILES})
    source_group("Structs" FILES ${struct_files})
    source_group("Enums" FILES ${enum_files})

    # build must occur after this two targets
    add_dependencies(${binding_target} ${ENGINE_CORE_LIB_TARGET} ${ENGINE_PLUGIN_JS_TARGET})

    # symlink generated binding module to executable directory
    set(SYMLINK_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    if(WIN32)
        set(SYMLINK_PATH "${SYMLINK_PATH}/$<CONFIG>/${binding_target}.dll")
    elseif(APPLE)
        set(SYMLINK_PATH "${SYMLINK_PATH}/$<CONFIG>/lib${binding_target}.dylib")
    else()
        set(SYMLINK_PATH "${SYMLINK_PATH}/$<CONFIG>/lib${binding_target}.so")
    endif()

    add_custom_command(TARGET ${binding_target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        $<TARGET_FILE:${binding_target}>
        ${SYMLINK_PATH}
        COMMENT "Creating symbolic link for ${binding_target}"
    )
endmacro()

add_js_binding(EngineCoreModule ${ENGINE_CORE_BINDING_SRC})
