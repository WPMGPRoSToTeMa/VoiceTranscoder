#!/bin/sh
#

cd VoiceCodecManager

echo "compile.sh got the control"
mkdir ../Release 2>/dev/null
rm -f ../Release/VoiceCodecMgr_i386.so 2>/dev/null

g++		-O3 \
		-funroll-loops \
		-fomit-frame-pointer \
		-fno-rtti \
		-s \
		-fno-stack-protector \
		-falign-functions=2 \
		-Wno-unknown-pragmas \
		-shared  \
		-static-libgcc \
		-fno-builtin \
		-fno-exceptions \
-I./hacker \
-I./hlsdk/common \
-I./hlsdk/dlls \
-I./hlsdk/engine \
-I./hlsdk/pm_shared \
-I./metamod \
-I./silk \
-I./speex \
build.cpp CRC32.cpp dllapi.cpp h_export.cpp interface.cpp \
meta_api.cpp sdk_util.cpp utlbuffer.cpp voice_codec_frame.cpp \
voicecodecmanager.cpp VoiceEncoder_Silk.cpp VoiceEncoder_Speex.cpp \
-ldl -lm \
-o ../Release/VoiceCodecMgr_i386.so \
-L./ -lSKP_SILK_SDK -lspeex

echo "leave this folder!"

cd ..