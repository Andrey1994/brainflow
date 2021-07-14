if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET (BOARD_CONTROLLER_NAME "BoardController")
    if (APPLE)
        SET (BOARD_CONTROLLER_COMPILED_NAME "libBoardController.dylib")
    elseif (UNIX)
        SET (BOARD_CONTROLLER_COMPILED_NAME "libBoardController.so")
    else ()
        SET (BOARD_CONTROLLER_COMPILED_NAME "BoardController.dll")
    endif (APPLE)
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (APPLE)
        SET (BOARD_CONTROLLER_NAME "BoardController")
        SET (BOARD_CONTROLLER_COMPILED_NAME "libBoardController.dylib")
    elseif (UNIX)
        SET (BOARD_CONTROLLER_NAME "BoardController")
        SET (BOARD_CONTROLLER_COMPILED_NAME "libBoardController.so")
    else ()
        SET (BOARD_CONTROLLER_NAME "BoardController32")
        SET (BOARD_CONTROLLER_COMPILED_NAME "BoardController32.dll")
    endif (APPLE)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

SET (BOARD_CONTROLLER_SRC
    ${CMAKE_HOME_DIRECTORY}/src/utils/timestamp.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/data_buffer.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/os_serial.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/os_serial_ioctl.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/serial.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/libftdi_serial.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/socket_client_tcp.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/socket_client_udp.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/socket_server_tcp.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/socket_server_udp.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/multicast_client.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/multicast_server.cpp
    ${CMAKE_HOME_DIRECTORY}/src/utils/broadcast_client.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/galea_serial.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/openbci_serial_board.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/openbci_wifi_shield_board.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/ganglion_wifi.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/cyton_wifi.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/cyton_daisy_wifi.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/ganglion.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/cyton.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/cyton_daisy.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/board_controller.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/board_info_getter.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/board.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/brainflow_boards.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/streaming_board.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/synthetic_board.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/dyn_lib_board.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/playback_file_board.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/galea.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/file_streamer.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/multicast_streamer.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/gtec/unicorn_board.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/neuromd/neuromd_board.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/neuromd/brainbit.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/neuromd/callibri.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/neuromd/callibri_eeg.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/neuromd/callibri_ecg.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/neuromd/callibri_emg.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/mit/fascia.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/neurosity/notion_osc.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/oymotion/gforce_pro.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/oymotion/gforce_dual.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/ironbci/ironbci.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/freeeeg32/freeeeg32.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/neuromd/brainbit_bled.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/muse/muse_s_bled.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/muse/muse_2_bled.cpp
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/ant_neuro/ant_neuro.cpp
)

include (${CMAKE_HOME_DIRECTORY}/src/board_controller/ant_neuro/build.cmake)
include (${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/ganglion_bglib/build.cmake)
include (${CMAKE_HOME_DIRECTORY}/src/board_controller/gtec/build.cmake)
include (${CMAKE_HOME_DIRECTORY}/src/board_controller/muse/muse_bglib/build.cmake)
include (${CMAKE_HOME_DIRECTORY}/src/board_controller/neuromd/build.cmake)

if (BUILD_OYMOTION_SDK)
    include (${CMAKE_HOME_DIRECTORY}/third_party/gForceSDKCXX/build.cmake)
endif (BUILD_OYMOTION_SDK)

add_library (
    ${BOARD_CONTROLLER_NAME} SHARED
    ${BOARD_CONTROLLER_SRC}
)

target_include_directories (
    ${BOARD_CONTROLLER_NAME} PRIVATE
    ${CMAKE_HOME_DIRECTORY}/third_party/
    ${CMAKE_HOME_DIRECTORY}/third_party/json
    ${CMAKE_HOME_DIRECTORY}/third_party/http
    ${CMAKE_HOME_DIRECTORY}/third_party/unicorn/inc
    ${CMAKE_HOME_DIRECTORY}/third_party/oscpp/include
    ${CMAKE_HOME_DIRECTORY}/src/utils/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/oymotion/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/gtec/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/neuromd/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/openbci/ganglion_bglib/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/neuromd/brainbit_bglib/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/muse/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/muse/muse_bglib/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/mit/inc
    ${CMAKE_HOME_DIRECTORY}/third_party/neurosdk/inc/types
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/neurosity/inc
    ${CMAKE_HOME_DIRECTORY}/third_party/gForceSDKCXX/src/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/ironbci/inc
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/freeeeg32/inc
    ${CMAKE_HOME_DIRECTORY}/third_party/ant_neuro
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/ant_neuro/inc
)

target_compile_definitions(${BOARD_CONTROLLER_NAME} PRIVATE -DNOMINMAX)

set_target_properties (${BOARD_CONTROLLER_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/compiled
)

if (USE_LIBFTDI)
    find_package (LibFTDI1 NO_MODULE)
    if (LibFTDI1_FOUND)
        # this line could SET up the directories if included before targets are defined
        #include ( ${LIBFTDI_USE_FILE} )
        # this works around a bug in some installations of libftdi++ 1.5
        # http://developer.intra2net.com/git/?p=libftdi;a=commit;h=cdb28383402d248dbc6062f4391b038375c52385
        SET (LIBFTDI_INCLUDE_DIRS ${LIBFTDI_INCLUDE_DIRS} ${LIBFTDI_INCLUDE_DIR}/../libftdi1)
        target_compile_definitions (${BOARD_CONTROLLER_NAME} PRIVATE ${LIBFTDI_DEFINITIONS} "USE_LIBFTDI")
        target_include_directories (${BOARD_CONTROLLER_NAME} PRIVATE ${LIBFTDI_INCLUDE_DIRS})
        target_link_directories (${BOARD_CONTROLLER_NAME} PRIVATE ${LIBFTDI_LIBRARY_DIRS})
        target_link_libraries (${BOARD_CONTROLLER_NAME} PRIVATE ${LIBFTDI_LIBRARIES})
    else (LibFTDI1_FOUND)
        message (FATAL_ERROR "USE_LIBFTDI SET but LibFTDI not found.")
    endif (LibFTDI1_FOUND)
endif (USE_LIBFTDI)

if (MSVC)
    add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/$<CONFIG>/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/board_controller/inc/board_controller.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/board_controller.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/board_controller/inc/board_info_getter.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/board_info_getter.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/utils/inc/shared_export_matlab.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/shared_export.h"
    )
endif (MSVC)
if (UNIX AND NOT ANDROID)
    add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/python-package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/julia-package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/java-package/brainflow/src/main/resources/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/csharp-package/brainflow/brainflow/${BOARD_CONTROLLER_COMPILED_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/board_controller/inc/board_controller.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/board_controller.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/board_controller/inc/board_info_getter.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/board_info_getter.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/utils/inc/shared_export_matlab.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/shared_export.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/src/utils/inc/brainflow_constants.h" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/inc/brainflow_constants.h"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/matlab-package/brainflow/lib/${BOARD_CONTROLLER_COMPILED_NAME}"
    )
endif (UNIX AND NOT ANDROID)

if (ANDROID)
    add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_HOME_DIRECTORY}/compiled/${BOARD_CONTROLLER_COMPILED_NAME}" "${CMAKE_HOME_DIRECTORY}/tools/jniLibs/${ANDROID_ABI}/${BOARD_CONTROLLER_COMPILED_NAME}"
    )
    if (LibFTDI1_FOUND)
        add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_HOME_DIRECTORY}/java-package/android/src/main/libs/${ANDROID_ABI}/"
        )
        foreach (DEPENDENCY_LIBRARY ${LIBFTDI_LIBRARIES})
            string (REGEX REPLACE "^-l" "" DEPENDENCY_LIBRARY ${DEPENDENCY_LIBRARY})
            if (DEPENDENCY_LIBRARY STREQUAL "log")
                continue ()
            endif (DEPENDENCY_LIBRARY STREQUAL "log")
            find_library (DEPENDENCY_LIBRARY_LOCATION NAMES ${DEPENDENCY_LIBRARY} HINTS ${LIBFTDI_LIBRARY_DIRS} / NO_CMAKE_PATH NO_CMAKE_FIND_ROOT_PATH)
            add_custom_command (TARGET ${BOARD_CONTROLLER_NAME} POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${DEPENDENCY_LIBRARY_LOCATION}" "${CMAKE_HOME_DIRECTORY}/tools/jniLibs/${ANDROID_ABI}/"
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${DEPENDENCY_LIBRARY_LOCATION}" "${CMAKE_HOME_DIRECTORY}/java-package/android/src/main/libs/${ANDROID_ABI}/"
            )
            unset (DEPENDENCY_LIBRARY_LOCATION CACHE)
        endforeach (DEPENDENCY_LIBRARY)
    endif (LibFTDI1_FOUND)
endif (ANDROID)

if (UNIX AND NOT ANDROID)
    target_link_libraries (${BOARD_CONTROLLER_NAME} PRIVATE pthread dl)
endif (UNIX AND NOT ANDROID)
if (ANDROID)
    find_library (log-lib log)
    target_link_libraries (${BOARD_CONTROLLER_NAME} PRIVATE log)
endif (ANDROID)

install (
    FILES
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/inc/board_controller.h
    ${CMAKE_HOME_DIRECTORY}/src/utils/inc/brainflow_constants.h
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/inc/board_info_getter.h
    ${CMAKE_HOME_DIRECTORY}/src/board_controller/inc/brainflow_input_params.h
    ${CMAKE_HOME_DIRECTORY}/src/utils/inc/shared_export.h
    ${CMAKE_HOME_DIRECTORY}/src/utils/inc/brainflow_array.h
    ${CMAKE_HOME_DIRECTORY}/src/utils/inc/brainflow_exception.h
    ${CMAKE_HOME_DIRECTORY}/third_party/json/json.hpp
    DESTINATION inc
)

install (
    TARGETS ${BOARD_CONTROLLER_NAME}
    EXPORT ${TARGETS_EXPORT_NAME}
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION inc
    ARCHIVE DESTINATION lib
)
