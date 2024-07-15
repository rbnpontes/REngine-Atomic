# Define source files
file (GLOB SOURCE_FILES *.cpp *.h )

if (WIN32)
    set (EXE_TYPE WIN32)
    set (SOURCE_FILES ${SOURCE_FILES} ${ENGINE_SOURCE_DIR}/Build/CMake/Modules/Engine.rc)
elseif(APPLE)
    #ICNS
    set(MACOSX_BUNDLE_ICON_FILE Engine.icns)
  if (NOT IOS)
    set(ENGINE_PLAYER_ICON ${ENGINE_SOURCE_DIR}/Build/CMake/Modules/Engine.icns)
  endif()
  set_source_files_properties(${ENGINE_PLAYER_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  set (EXE_TYPE MACOSX_BUNDLE)
endif()

if (ANDROID)
  add_library(${ENGINE_PLAYER_TARGET} SHARED ${SOURCE_FILES} ${ENGINE_SOURCE_DIR}/Source/ThirdParty/SDL/src/main/android/SDL_android_main.c)
else()
  # iOS AtomicPlayer currently disabled
  if (NOT IOS)
    add_executable(${ENGINE_PLAYER_TARGET} ${EXE_TYPE} ${SOURCE_FILES} ${ENGINE_PLAYER_ICON})
  endif()
endif()

if (NOT IOS)
    target_link_libraries(${ENGINE_PLAYER_TARGET} AtomicJS ${ENGINE_PLAYER_TARGET_LIB} AtomicPlayerJS ${ENGINE_CORE_LIB_TARGET})
endif()

if (APPLE)
  if (NOT IOS)
    set (TARGET_PROPERTIES MACOSX_BUNDLE_INFO_PLIST MacOSXBundleInfo.plist.template)
  else()
#    set_target_properties(AtomicPlayer PROPERTIES
#      MACOSX_BUNDLE_GUI_IDENTIFIER "com.atomicgameengine.atomicplayer"
#      XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
#      XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
#      XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
#      XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 9.3
#      XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2" # iPhone/iPad
#      XCODE_ATTRIBUTE_COMBINE_HIDPI_IMAGES NO
#    )

#    set (TARGET_PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${ENGINE_SOURCE_DIR}/Build/CMake/Modules/iOSBundleInfo.plist.template)
  endif()
endif()


if (TARGET_PROPERTIES)
  if (NOT IOS)
      set_target_properties (${ENGINE_PLAYER_TARGET} PROPERTIES ${TARGET_PROPERTIES})
  endif()
endif ()

vs_add_to_grp(${ENGINE_PLAYER_TARGET} "${VS_GRP_ENGINE_SRC}")