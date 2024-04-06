include(FetchContent)

FetchContent_Declare(
  tracy
  GIT_REPOSITORY https://github.com/wolfpld/tracy.git
  GIT_TAG        v0.10
)

FetchContent_MakeAvailable(tracy)

set(TRACY_ON_DEMAND TRUE)