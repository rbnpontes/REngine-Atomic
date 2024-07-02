@echo OFF

call %0\..\SetupVSEnvironment.bat %1

if not defined ENGINE_CMAKE_GENERATOR (
  @echo Problem setting up %1 compilation environment
  exit
)

@echo OFF

set ENGINE_CMAKE=cmake
set ENGINE_ROOT=%~2
set ENGINE_SOLUTION_PATH=%~3
set ENGINE_CMAKE_FLAGS="%4"

%ENGINE_CMAKE% -E make_directory "%ENGINE_SOLUTION_PATH%"
%ENGINE_CMAKE% -E chdir "%ENGINE_SOLUTION_PATH%" %ENGINE_CMAKE% "%ENGINE_ROOT%" %ENGINE_CMAKE_FLAGS% -G %ENGINE_CMAKE_GENERATOR%
