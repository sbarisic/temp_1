#
# Copyright (c) .NET Foundation and Contributors
# See LICENSE file in the project root for full license information.
#

########################################################################################
# make sure that a valid path is set bellow                                            #
# this is an Interop module so this file should be placed in the CMakes module folder  #
# usually CMake\Modules                                                                #
########################################################################################

# native code directory
set(BASE_PATH_FOR_THIS_MODULE ${PROJECT_SOURCE_DIR}/InteropAssemblies/ESP32CAN)


# set include directories
list(APPEND ESP32CAN_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src/CLR/Core)
list(APPEND ESP32CAN_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src/CLR/Include)
list(APPEND ESP32CAN_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src/HAL/Include)
list(APPEND ESP32CAN_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src/PAL/Include)
list(APPEND ESP32CAN_INCLUDE_DIRS ${BASE_PATH_FOR_THIS_MODULE})


# source files
set(ESP32CAN_SRCS

    ESP32CAN.cpp


    ESP32CAN_ESP32CAN_CAN_mshl.cpp
    ESP32CAN_ESP32CAN_CAN.cpp

)

foreach(SRC_FILE ${ESP32CAN_SRCS})

    set(ESP32CAN_SRC_FILE SRC_FILE-NOTFOUND)

    find_file(ESP32CAN_SRC_FILE ${SRC_FILE}
        PATHS
	        ${BASE_PATH_FOR_THIS_MODULE}
	        ${TARGET_BASE_LOCATION}
            ${PROJECT_SOURCE_DIR}/src/ESP32CAN

	    CMAKE_FIND_ROOT_PATH_BOTH
    )

    if (BUILD_VERBOSE)
        message("${SRC_FILE} >> ${ESP32CAN_SRC_FILE}")
    endif()

    list(APPEND ESP32CAN_SOURCES ${ESP32CAN_SRC_FILE})

endforeach()

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(ESP32CAN DEFAULT_MSG ESP32CAN_INCLUDE_DIRS ESP32CAN_SOURCES)
