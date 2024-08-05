# TODO: Look into the application-extension flag for iOS
#if (IOS)
#    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fapplication-extension")
#    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fapplication-extension")
#endif()

set (CS_DIR "${ENGINE_SOURCE_DIR}/Artifacts/Build/Source/Generated/CSharp/Packages/")

file (GLOB CSHARP_BINDINGS_SOURCE ${CS_DIR}/Atomic/Native/*.cpp ${CS_DIR}/EngineCore/Native/*.h
                                  ${CS_DIR}/${ENGINE_NET_NATIVE_TARGET}/Native/*.cpp ${CS_DIR}/${ENGINE_NET_NATIVE_TARGET}/Native/*.h
                                  ${CS_DIR}/${ENGINE_NET_SCRIPT_TARGET}/Native/*.cpp ${CS_DIR}/${ENGINE_NET_SCRIPT_TARGET}/Native/*.h
                                  ${CS_DIR}/${ENGINE_APP_TARGET}/Native/*.cpp ${CS_DIR}/${ENGINE_APP_TARGET}/Native/*.h
                                  ${CS_DIR}/${ENGINE_PLAYER_TARGET}/Native/*.cpp ${CS_DIR}/${ENGINE_PLAYER_TARGET}/Native/*.h )

file (GLOB SOURCE_FILES *.cpp *.h)

if (NOT IOS AND NOT ANDROID AND NOT WEB)
    file (GLOB DESKTOP_SOURCE_FILES Desktop/*.cpp Desktop/*.h)
    set (SOURCE_FILES ${SOURCE_FILES} ${DESKTOP_SOURCE_FILES})
endif()

if (ANDROID)
	set (SOURCE_FILES ${SOURCE_FILES} ${ENGINE_SOURCE_DIR}/Source/ThirdParty/SDL/src/main/android/SDL_android_main.c)
endif()

add_library(${ENGINE_NET_NATIVE_TARGET} SHARED ${SOURCE_FILES} ${CSHARP_BINDINGS_SOURCE})
if (ENGINE_DESKTOP)
    add_dependencies(${ENGINE_NET_NATIVE_TARGET} EngineToolCheckScripts)
endif ()

target_link_libraries(${ENGINE_NET_NATIVE_TARGET} 
    ${ENGINE_APP_TARGET} 
    ${ENGINE_NET_SCRIPT_BINDINGS_TARGET} 
    ${ENGINE_NET_SCRIPT_TARGET} 
    ${ENGINE_JS_TARGET} 
    ${ENGINE_PLAYER_TARGET_LIB} 
    ${ENGINE_PLAYER_JS_TARGET} 
    ${ENGINE_CORE_LIB_TARGET})
target_include_directories(${ENGINE_NET_NATIVE_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (APPLE)
    if (NOT IOS)
        target_link_libraries(${ENGINE_NET_NATIVE_TARGET} "-stdlib=libc++ -framework AudioToolbox -framework Carbon -framework Cocoa -framework CoreAudio -framework CoreVideo -framework ForceFeedback -framework IOKit -framework OpenGL -framework CoreServices -framework Security -framework SystemConfiguration")
    else()
        set_target_properties(${ENGINE_NET_NATIVE_TARGET} 
        PROPERTIES
            FRAMEWORK TRUE
            MACOSX_FRAMEWORK_IDENTIFIER com.rengine.netframework
            MACOSX_FRAMEWORK_INFO_PLIST ${ENGINE_SOURCE_DIR}/Build/CMake/IOS/${ENGINE_NET_NAME}.framework.plist
            # PUBLIC_HEADER dynamicFramework.h
            XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
        )
        target_link_libraries(${ENGINE_NET_NATIVE_TARGET} "-framework AudioToolbox -framework AVFoundation -framework CoreAudio -framework CoreGraphics -framework Foundation -framework OpenGLES -framework QuartzCore -framework UIKit -framework CoreMotion -framework GameController")
    endif()
endif()

if (WIN32)
    set (NET_NATIVE_PLATFORM Windows)
elseif(APPLE)
    if (IOS)
        set (NET_NATIVE_PLATFORM iOS)
    else()
        set (NET_NATIVE_PLATFORM Mac)
    endif()
elseif(LINUX)
    set (NET_NATIVE_PLATFORM Linux)
elseif(ANDROID)
    set (NET_NATIVE_PLATFORM Android)
endif()

set (NET_NATIVE_DIR "${ENGINE_SOURCE_DIR}/Artifacts/${ENGINE_NET_NAME}/$<$<CONFIG:debug>:Debug>$<$<CONFIG:release>:Release>/Native/${NET_NATIVE_PLATFORM}")

if (NOT IOS)
    add_custom_command( TARGET ${ENGINE_NET_NATIVE_TARGET} POST_BUILD
                        COMMAND "${CMAKE_COMMAND}" ARGS -E make_directory "\"${NET_NATIVE_DIR}\""
                        COMMAND "${CMAKE_COMMAND}" ARGS -E copy_if_different \"$<TARGET_FILE:${ENGINE_NET_NATIVE_TARGET}>\" "\"${NET_NATIVE_DIR}\"" )
else()
    # See BuildIOS.js for handling of the NETNative framework, including install_name_tool modification, etc
    # Handling it here was causing issues with certain builds of XCode and where it thought the TARGET_FILE was...
endif()

vs_add_to_grp(${ENGINE_NET_NATIVE_TARGET} "${VS_GRP_ENGINE_SRC}")