include(CheckCXXSourceRuns)

set(CMAKE_REQUIRED_FLAGS "-msse2")

# If this compile check fails, we will not be able to use SSE2 instructions
check_cxx_source_runs("
    #include <emmintrin.h>
    int main() {
        __m128d x;
        x = _mm_set1_pd(0);
        return 0;
    }
" ENGINE_SSE)

macro(GroupSources curdir)
    if (WIN32)
        file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${curdir} ${CMAKE_CURRENT_SOURCE_DIR}/${curdir}/*)
        foreach (child ${children})
            if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${curdir}/${child})
                if ("${curdir}" STREQUAL "")
                    GroupSources(${child})
                else ()
                    GroupSources(${curdir}/${child})
                endif ()
            else ()
                string(REPLACE "/" "\\" groupname ${curdir})
                source_group(${groupname} FILES ${CMAKE_CURRENT_SOURCE_DIR}/${curdir}/${child})
            endif ()
        endforeach ()
    endif ()
endmacro()

if (NOT CMAKE_CROSSCOMPILING AND ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(LINUX TRUE CACHE BOOL "Indicates if host is Linux.")
endif ()

