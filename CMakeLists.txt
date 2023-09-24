# cmake -B build -DCMAKE_SYSTEM_NAME=WindowsStore "-DCMAKE_SYSTEM_VERSION=10.0"
cmake_minimum_required(VERSION 3.10)

set(APP_NAME HelloCpp)

project(${APP_NAME})

# Patch up the generated vcxproj
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/Directory.Build.props.in"
  "${CMAKE_CURRENT_BINARY_DIR}/Directory.Build.props"
  COPYONLY
)
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/Directory.Build.targets.in"
  "${CMAKE_CURRENT_BINARY_DIR}/Directory.Build.targets"
  COPYONLY
)

set(DEFAULT_TARGET_PROPS
    # Require C++17.
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED TRUE
    # Prefer C11, but support C99 and earlier when possible.
    C_STANDARD 11)

macro(source_group_by_dir proj_dir source_files)
    if(MSVC OR APPLE)
        get_filename_component(sgbd_cur_dir ${proj_dir} ABSOLUTE)
        foreach(sgbd_file ${${source_files}})
            get_filename_component(sgbd_abs_file ${sgbd_file} ABSOLUTE)
            file(RELATIVE_PATH sgbd_fpath ${sgbd_cur_dir} ${sgbd_abs_file})
            string(REGEX REPLACE "\(.*\)/.*" \\1 sgbd_group_name ${sgbd_fpath})
            string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
            string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
            if(sgbd_nogroup)
                set(sgbd_group_name "\\")
            endif(sgbd_nogroup)
            source_group(${sgbd_group_name} FILES ${sgbd_file})
        endforeach(sgbd_file)
    endif(MSVC OR APPLE)
endmacro(source_group_by_dir)

set(HEADER_FILES
  pch.h
  App.h
  OpenGLESPage.h
  )
  
set(SOURCE_FILES
  pch.cpp
  App.cpp
  OpenGLESPage.cpp
  )

set(XAML_FILES
  App.xaml
  OpenGLESPage.xaml
  )

set(IDL_FILES App.idl OpenGLESPage.idl)
  
file(GLOB_RECURSE ASSET_FILES
    Assets/*.png
    )

set(CONTENT_FILES
    packages.config
    Package.appxmanifest
    )


file(TO_NATIVE_PATH "${CMAKE_CURRENT_LIST_DIR}/App.xaml" APP_XAML_FULL_PATH)

set_property(
    SOURCE App.h App.cpp App.idl
    PROPERTY VS_SETTINGS
    "DependentUpon=${APP_XAML_FULL_PATH}"
)

file(TO_NATIVE_PATH "${CMAKE_CURRENT_LIST_DIR}/OpenGLESPage.xaml" OPENGLESPAGE_XAML_FULL_PATH)
set_property(
    SOURCE OpenGLESPage.h OpenGLESPage.cpp OpenGLESPage.idl
    PROPERTY VS_SETTINGS
    "DependentUpon=${OPENGLESPAGE_XAML_FULL_PATH}"
)

set(RESOURCE_FILES ${CONTENT_FILES} ${ASSET_FILES})

set_property(SOURCE ${ASSET_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)
set_property(SOURCE ${ASSET_FILES} PROPERTY VS_DEPLOYMENT_LOCATION "Assets")

set_property(SOURCE "App.xaml" PROPERTY VS_XAML_TYPE "ApplicationDefinition")

set(APP_ALL_SOURCES ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES} ${XAML_FILES} ${IDL_FILES})
source_group_by_dir(${CMAKE_CURRENT_SOURCE_DIR} APP_ALL_SOURCES)

add_executable(${APP_NAME} WIN32 ${APP_ALL_SOURCES})

# set_target_properties(${APP_NAME} PROPERTIES ${DEFAULT_TARGET_PROPS}) 
set_target_properties(${APP_NAME} PROPERTIES VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION "10.0.19041.0")

set_target_properties(
  ${APP_NAME}
  PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${APP_NAME}"
  # ----- C++/WinRT, Windows App SDK, and WinUI stuff starts here -----
  VS_GLOBAL_CppWinRTOptimized true
  VS_GLOBAL_CppWinRTRootNamespaceAutoMerge true
  VS_GLOBAL_CppWinRTGenerateWindowsMetadata true
  VS_GLOBAL_MinimalCoreWin true
  VS_GLOBAL_AppContainerApplication true
  # VS_GLOBAL_ApplicationType "Windows Store"
  VS_GLOBAL_RootNameSpace "AxmolAppWinRT" # this is important for cppwirnt to fix Generated Files\XamlMetaDataProvider.idl(4): error MIDL2025: [msg]syntax error [context]: expecting NamespaceTag near "{"
)

# cmake write <Midl> will cause directory error
# <Midl>
# <AdditionalIncludeDirectories>%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
# <OutputDirectory>$(ProjectDir)/$(IntDir)</OutputDirectory>
# <HeaderFileName>%(Filename).h</HeaderFileName>
# <TypeLibraryName>%(Filename).tlb</TypeLibraryName>
# <InterfaceIdentifierFileName>%(Filename)_i.c</InterfaceIdentifierFileName>
# <ProxyFileName>%(Filename)_p.c</ProxyFileName>
# </Midl>

target_include_directories(${APP_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# cppwinrt or webview2 require nuget
if(WINDOWS_STORE)
find_program(NUGET_EXE NAMES nuget 
    PATHS ${_AX_ROOT}/tools/external/nuget)
    
if(NOT NUGET_EXE)
    message("NUGET.EXE not found.")
    message(FATAL_ERROR "Please run setup.ps1 again to download NUGET.EXE, and run CMake again.")
endif()
endif()

if(WINDOWS_STORE)
exec_program(${NUGET_EXE}
        ARGS install "Microsoft.Windows.CppWinRT" -Version 2.0.230706.1 -ExcludeVersion -OutputDirectory "\"${CMAKE_BINARY_DIR}/packages\"")
set_target_properties(${APP_NAME} PROPERTIES
    VS_PROJECT_IMPORT ${CMAKE_BINARY_DIR}/packages/Microsoft.Windows.CppWinRT/build/native/Microsoft.Windows.CppWinRT.props
)
target_link_libraries(${APP_NAME} ${CMAKE_BINARY_DIR}/packages/Microsoft.Windows.CppWinRT/build/native/Microsoft.Windows.CppWinRT.targets)
endif()

