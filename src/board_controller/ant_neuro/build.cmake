if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (UNIX AND NOT APPLE AND NOT ANDROID)
        SET (ANT_LIB_NAME "libeego-SDK.so") 
    endif (UNIX AND NOT APPLE AND NOT ANDROID)
    if (MSVC)
        SET (ANT_LIB_NAME "eego-SDK.dll")
    endif (MSVC)
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (UNIX AND NOT APPLE)
        SET (ANT_LIB_NAME "libeego-SDK32.so") 
    endif (UNIX AND NOT APPLE)
    if (MSVC)
        SET (ANT_LIB_NAME "eego-SDK32.dll")
    endif (MSVC)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

if (MSVC OR UNIX AND NOT APPLE AND NOT ANDROID)
    SET (BOARD_CONTROLLER_SRC ${BOARD_CONTROLLER_SRC}
        ${CMAKE_HOME_DIRECTORY}/third_party/ant_neuro/eemagine/sdk/wrapper.cc)
endif (MSVC OR UNIX AND NOT APPLE AND NOT ANDROID)

if (MSVC)
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/ant_neuro/windows/${ANT_LIB_NAME}" DESTINATION "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/ant_neuro/windows/${ANT_LIB_NAME}" DESTINATION "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/ant_neuro/windows/${ANT_LIB_NAME}" DESTINATION "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/ant_neuro/windows/${ANT_LIB_NAME}" DESTINATION "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/ant_neuro/windows/${ANT_LIB_NAME}" DESTINATION "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/")
endif (MSVC)
if (UNIX AND NOT APPLE AND NOT ANDROID)
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/ant_neuro/linux/${ANT_LIB_NAME}" DESTINATION "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/ant_neuro/linux/${ANT_LIB_NAME}" DESTINATION "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/ant_neuro/linux/${ANT_LIB_NAME}" DESTINATION "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/ant_neuro/linux/${ANT_LIB_NAME}" DESTINATION "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/")
    file (COPY "${CMAKE_HOME_DIRECTORY}/third_party/ant_neuro/linux/${ANT_LIB_NAME}" DESTINATION "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/")
endif (UNIX AND NOT APPLE AND NOT ANDROID)

if (MSVC)
    install (
        FILES
        ${CMAKE_HOME_DIRECTORY}/third_party/ant_neuro/windows/${ANT_LIB_NAME}
        DESTINATION lib
    )
endif (MSVC)
if (UNIX AND NOT APPLE AND NOT ANDROID)
    install (
        FILES
        ${CMAKE_HOME_DIRECTORY}/third_party/ant_neuro/linux/${ANT_LIB_NAME}
        DESTINATION lib
    )
endif (UNIX AND NOT APPLE AND NOT ANDROID)
