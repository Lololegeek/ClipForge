cmake_minimum_required(VERSION 3.24)
project(ClipForge VERSION 0.4.0 LANGUAGES CXX)
if(CMAKE_CXX_CL_SHOWINCLUDES_PREFIX MATCHES "inclusion du fichier")
  # Some French-only MSVC kits emit UTF-8 but CMake probes the prefix as OEM text.
  set(CMAKE_CL_SHOWINCLUDES_PREFIX "Remarque : inclusion du fichier :  ")
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
if(NOT MSVC OR NOT WIN32)
  message(FATAL_ERROR "ClipForge requires Windows x64 and MSVC")
endif()
find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Widgets Quick QuickControls2 Multimedia Network Sql Test)
set(OBS_ROOT "${CMAKE_SOURCE_DIR}/.deps/obs" CACHE PATH "OBS portable runtime")
set(OBS_SOURCE "${CMAKE_SOURCE_DIR}/.deps/obs-source/obs-studio-32.2.2-sources" CACHE PATH "Matching OBS source")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/obs-include")
file(WRITE "${CMAKE_BINARY_DIR}/obs-include/obsconfig.h" "#pragma once\n#define OBS_DATA_PATH \"data\"\n#define OBS_PLUGIN_PATH \"obs-plugins\"\n#define OBS_RELEASE_CANDIDATE 0\n#define OBS_BETA 0\n")
add_library(obs SHARED IMPORTED)
set_target_properties(obs PROPERTIES IMPORTED_IMPLIB "${OBS_ROOT}/obs.lib" IMPORTED_LOCATION "${OBS_ROOT}/bin/64bit/obs.dll" INTERFACE_INCLUDE_DIRECTORIES "${OBS_SOURCE}/libobs;${CMAKE_BINARY_DIR}/obs-include")
add_compile_options(/utf-8 /W4 /permissive-)
add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)
add_library(clipforge_core STATIC
 src/settings/Settings.cpp src/settings/Settings.h
 src/capture/ObsEngine.cpp src/capture/ObsEngine.h
 src/capture/sources/ObsSources.cpp src/capture/replay/ObsReplay.cpp src/capture/obs/ObsLifecycle.cpp
 src/platform/WindowCatalog.cpp src/platform/WindowCatalog.h
 src/hotkeys/GlobalHotkeys.cpp src/hotkeys/GlobalHotkeys.h
 src/library/ClipModel.cpp src/library/ClipModel.h
 src/export/MediaJobs.cpp src/export/MediaJobs.h
 src/editor/EditProject.cpp src/editor/EditProject.h
 src/playback/PlaybackAudio.cpp src/playback/PlaybackAudio.h)
target_include_directories(clipforge_core PUBLIC src)
target_link_libraries(clipforge_core PUBLIC Qt6::Core Qt6::Gui Qt6::Sql Qt6::Multimedia obs user32 shell32 psapi ole32)
qt_add_executable(ClipForge WIN32 src/app/main.cpp src/app/Controller.cpp src/app/Controller.h
 src/app/Updater.cpp src/app/Updater.h src/app/Overlay.cpp src/app/Overlay.h src/app/CaptureActions.cpp src/app/LibraryActions.cpp src/app/EditActions.cpp src/app/GameProfiles.cpp)
target_compile_definitions(ClipForge PRIVATE CF_VERSION="${PROJECT_VERSION}")
configure_file(packaging/windows.rc.in "${CMAKE_BINARY_DIR}/windows.rc" @ONLY)
set_source_files_properties("${CMAKE_BINARY_DIR}/windows.rc" PROPERTIES
  OBJECT_DEPENDS "${CMAKE_SOURCE_DIR}/assets/ClipForge.ico")
target_sources(ClipForge PRIVATE "${CMAKE_BINARY_DIR}/windows.rc")
target_link_libraries(ClipForge PRIVATE clipforge_core Qt6::Widgets Qt6::Quick Qt6::QuickControls2 Qt6::Multimedia Qt6::Network)
qt_add_resources(ClipForge ui PREFIX "/" FILES
 ui/App.qml ui/theme/Theme.qml ui/theme/qmldir
 ui/components/ActionButton.qml ui/components/Panel.qml ui/components/CField.qml ui/components/CCombo.qml ui/components/CSlider.qml ui/components/CDialog.qml ui/components/CMenu.qml ui/components/CMenuItem.qml ui/components/CMenuSeparator.qml
 ui/pages/HomePage.qml ui/pages/ClipsPage.qml ui/pages/RecordPage.qml
 ui/pages/EditorPage.qml ui/pages/GamesPage.qml ui/pages/SettingsPage.qml
 assets/icons/home.svg assets/icons/clips.svg assets/icons/record.svg
 assets/icons/edit.svg assets/icons/games.svg assets/icons/settings.svg assets/icons/clipforge-mark.svg
 assets/ClipForge.ico)
enable_testing()
qt_add_executable(core_tests tests/CoreTests.cpp)
target_link_libraries(core_tests PRIVATE clipforge_core Qt6::Test)
add_test(NAME core_tests COMMAND core_tests)
qt_add_executable(media_tests tests/MediaTests.cpp)
target_link_libraries(media_tests PRIVATE clipforge_core Qt6::Test Qt6::Multimedia)
add_executable(ClipForgeFixture WIN32 tests/CaptureFixture.cpp)
target_link_libraries(ClipForgeFixture PRIVATE user32 gdi32)
set_tests_properties(core_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
set_tests_properties(core_tests PROPERTIES
 ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_PLUGIN_PATH=${Qt6_DIR}/../../../plugins"
 ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${OBS_ROOT}/bin/64bit;PATH=path_list_prepend:${Qt6_DIR}/../../../bin")
install(TARGETS ClipForge RUNTIME DESTINATION .)
