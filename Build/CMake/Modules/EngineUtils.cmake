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

macro(group_sources base_path cur_dir)
    # Get the list of files and directories in the current directory
    file(GLOB children RELATIVE ${base_path}/${cur_dir} ${base_path}/${cur_dir}/*)
    
    foreach (child ${children})
        # Check if the child is a directory
        if (IS_DIRECTORY ${base_path}/${cur_dir}/${child})
            # If cur_dir is empty (root), call with the child directory directly
            if ("${cur_dir}" STREQUAL "")
                group_sources(${base_path} ${child})
            else()
                # Recursively call for subdirectories, updating cur_dir
                group_sources(${base_path} ${cur_dir}/${child})
            endif ()
        else ()
            # Replace "/" with "\\" to make it compatible with Visual Studio filters
            string(REPLACE "/" "\\" groupname ${cur_dir})
            
            # Add the file to the appropriate source group
            source_group(${groupname} FILES ${base_path}/${cur_dir}/${child})
        endif ()
    endforeach ()
endmacro()

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

