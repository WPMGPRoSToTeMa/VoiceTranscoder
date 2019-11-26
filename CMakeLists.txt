cmake_minimum_required(VERSION 3.3)

if(POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
endif()
cmake_policy(SET CMP0063 NEW)
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

project(VoiceTranscoder CXX)

if(CMAKE_SIZEOF_VOID_P EQUAL 4 AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(i386|i486|i586|i686|x86_64|AMD64)$")
    # OK
else()
    message(FATAL_ERROR "This project can be compiled only in x86 mode on x86/x64 processors (got ${CMAKE_SIZEOF_VOID_P} and ${CMAKE_SYSTEM_PROCESSOR})")
endif()

if(NOT DEFINED CMAKE_POSITION_INDEPENDENT_CODE)
    SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

add_subdirectory(VoiceCodecs/Opus)
add_subdirectory(VoiceCodecs/SILK)
add_subdirectory(VoiceCodecs/Speex)

add_library(${PROJECT_NAME} MODULE
    API.h API.cpp
    Main.h Main.cpp
    ThreadMode.h ThreadMode.cpp
    VoiceCodec.h
    VoiceCodec_SILK.h VoiceCodec_SILK.cpp
    VoiceCodec_Speex.h VoiceCodec_Speex.cpp
    VoiceTranscoderAPI.h
    
    Hashers/CRC32.h Hashers/CRC32.cpp

    MultiThreading/Mutex.h MultiThreading/Mutex.cpp
    MultiThreading/Signal.h MultiThreading/Signal.cpp
    MultiThreading/Thread.h MultiThreading/Thread.cpp

    Utility/AnyPointer.h
    Utility/BinaryPattern.h Utility/BinaryPattern.cpp
    Utility/Buffer.h Utility/Buffer.cpp
    Utility/EngineUTIL.h Utility/EngineUTIL.cpp
    Utility/FunctionHook.h
    Utility/FunctionHook_Beginning.h Utility/FunctionHook_Beginning.cpp
    Utility/GoldSrcEngineStructs.h
    Utility/JmpOpcode.h Utility/JmpOpcode.cpp
    Utility/MemoryUnlocker.h Utility/MemoryUnlocker.cpp
    Utility/MetaUTIL.h Utility/MetaUTIL.cpp
    Utility/Module.h Utility/Module.cpp
    Utility/Optional.h
    Utility/Section.h Utility/Section.cpp
    Utility/SteamID.h Utility/SteamID.cpp
    Utility/UtilFunctions.h Utility/UtilFunctions.cpp
    Utility/UtilTypes.h
)

target_include_directories(${PROJECT_NAME} PRIVATE
    hlsdk/common
    hlsdk/dlls
    hlsdk/engine
    hlsdk/pm_shared

    metamod

    Hashers
    MultiThreading
    Utility
)

if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
foreach(config ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${config} config)
    set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${config} "${CMAKE_BINARY_DIR}")
endforeach()

target_link_libraries(${PROJECT_NAME} PRIVATE Opus SILK Speex)

if(NOT MSVC)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        -static-libstdc++
        # Gold linker does not leave symbols from libstdc++ when using --exclude-libs
        # and also does not emit undefined references to dl* functions when using --no-undefined
        -fuse-ld=gold
        # Assume libstdc++ is not compiled with -flto, in case of -flto we should
        # either compile libstdc++ with no exports (AFAIK it is not currently possible)
        # or use -Wl,--version-script with the list of exports.
        -Wl,--exclude-libs,libstdc++
        -Wl,--no-undefined -Wl,--no-allow-shlib-undefined

        Threads::Threads -ldl
    )
endif()