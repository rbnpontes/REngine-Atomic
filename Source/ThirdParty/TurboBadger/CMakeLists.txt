if(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -Wall -Wextra")
    add_definitions(-DMACOSX)
endif(APPLE)

add_definitions(-DTB_FONT_RENDERER_FREETYPE)
#add_definitions(-DTB_FONT_RENDERER_STB)

# Define source files
file (GLOB C_FILES *.cpp image/*.cpp animation/*.cpp parser/*.cpp renderers/*.cpp utf8/*.cpp)
file (GLOB H_FILES *.h image/*.h animation/*.h parser/*.h renderers/*.h utf8/*.h)
set (SOURCE_FILES ${C_FILES} ${H_FILES})

add_library(TurboBadger STATIC ${SOURCE_FILES})

target_link_libraries(TurboBadger PRIVATE SDL2::SDL2-static FreeType)
set(TURBO_BADGER_PRIVATE_FLAGS)
target_include_directories(TurboBadger SYSTEM BEFORE PUBLIC .)
if (NOT MSVC)
    list(APPEND TURBO_BADGER_PRIVATE_FLAGS "-std=c++11")
endif ()

if(ANDROID)
    list(APPEND TURBO_BADGER_PRIVATE_FLAGS "-Wno-format-security")
endif()

target_compile_options(TurboBadger PRIVATE ${TURBO_BADGER_PRIVATE_FLAGS})

vs_add_to_grp(TurboBadger "${VS_GRP_THIRD_PARTY}")