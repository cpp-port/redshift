cmake_minimum_required(VERSION 3.16)
project(redshift CXX)


list(APPEND library_references
   )


list(APPEND library_source
   redshift.cpp
   colorramp.cpp

   #src/writers.cpp


   )
if (WIN32)

   list(APPEND library_source
      gamma-w32gdi.cpp
      )

endif ()

if(${LINUX} OR ${__BSD__})

   list(APPEND library_source
      gamma-randr.cpp
      )

endif ()


add_library(${PROJECT_NAME} SHARED ${library_source})
add_library(static_${PROJECT_NAME} STATIC ${library_source})
if(${LINUX})
   target_compile_definitions(${PROJECT_NAME} PRIVATE LINUX)
endif()

#target_precompile_headers(${PROJECT_NAME} PRIVATE framework.h)
#target_precompile_headers(static_${PROJECT_NAME} PRIVATE framework.h)

#if (NOT ${NO_PRECOMPILED_HEADER})
#
#   target_precompile_headers(${PROJECT_NAME} PRIVATE framework.h)
#   target_precompile_headers(static_${PROJECT_NAME} PRIVATE framework.h)
#
#endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_CONFIG_H)
target_compile_definitions(static_${PROJECT_NAME} PRIVATE HAVE_CONFIG_H)


if (MSVC)
   target_compile_options(${PROJECT_NAME} PRIVATE "/MD$<$<CONFIG:Debug>:d>")
   target_link_options(${PROJECT_NAME} PRIVATE "/DEFAULTLIB:msvcrt$<$<CONFIG:Debug>:d>")
   target_link_options(${PROJECT_NAME} PRIVATE "/SUBSYSTEM:WINDOWS")
   target_link_libraries(${PROJECT_NAME} PRIVATE ${library_references})
   target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 winmm shlwapi)
elseif(${LINUX} OR ${__BSD__})

   target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
   #set_target_properties(${PROJECT_NAME} PROPERTIES CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)





   pkg_check_modules(${PROJECT_NAME}_PKGCONFIG xcb xcb-randr)

   target_compile_options(${PROJECT_NAME} PRIVATE -fpermissive ${${PROJECT_NAME}_PKGCONFIG_CFLAGS})
   target_compile_options(static_${PROJECT_NAME} PRIVATE -fpermissive ${${PROJECT_NAME}_PKGCONFIG_CFLAGS})
   target_compile_definitions(static_${PROJECT_NAME} PRIVATE CUBE)

   target_link_options(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_PKGCONFIG_LDFLAGS})
   target_link_options(static_${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_PKGCONFIG_LDFLAGS})
   #target_link_options(${PROJECT_NAME} PRIVATE "-Wl,-rpath=\$ORIGIN")
   target_link_libraries(${PROJECT_NAME} PRIVATE z ${${PROJECT_NAME}_PKGCONFIG_LIBRARIES})
   target_link_libraries(${PROJECT_NAME} PRIVATE ${library_references} ${${PROJECT_NAME}_PKGCONFIG_LIBRARIES})
   target_link_options(${PROJECT_NAME} PRIVATE -Wl,-z,defs)
   # target_link_options(static_${PROJECT_NAME} PRIVATE --whole-archive)


endif ()


list(APPEND library_include_directories ${CMAKE_CURRENT_SOURCE_DIR}/../..)
list(APPEND library_include_directories ${CMAKE_CURRENT_SOURCE_DIR}/..)
list(APPEND library_include_directories ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND library_include_directories ${CMAKE_CURRENT_SOURCE_DIR}/include)
list(APPEND library_include_directories ${CMAKE_CURRENT_SOURCE_DIR}/../../../operating_system/operating_system-${PLATFORM_NAME}/configuration/${PROJECT_NAME})


target_include_directories(${PROJECT_NAME} PRIVATE ${library_include_directories})
target_include_directories(static_${PROJECT_NAME} PRIVATE ${library_include_directories})


target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)


target_compile_definitions(${PROJECT_NAME} PRIVATE _${PROJECT_NAME}_project)



