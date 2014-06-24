#!/bin/sh
#

echo "compile.sh got the control"
mkdir Release 2>/dev/null
rm -f Release/vtc.so 2>/dev/null

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
		-fpermissive \
		-fvisibility=hidden \
-I./hacker \
-I./hlsdk/common \
-I./hlsdk/dlls \
-I./hlsdk/engine \
-I./hlsdk/pm_shared \
-I./metamod \
-I./silk \
-I./speex \
-I./DprotoAPI \
Build.cpp CRC32.cpp DllAPI.cpp DProtoAPI.cpp EngineFuncs.cpp HExport.cpp \
MetaAPI.cpp Player.cpp SdkUTIL.cpp UtlBuffer.cpp \
VoiceTranscoder.cpp VoiceCodec_Silk.cpp VoiceCodec_Speex.cpp Logger.cpp Logging.cpp MetaUtil.cpp \
-ldl -lm \
-o Release/vtc.so \
-L./ -lSKP_SILK_SDK -lspeex

if [ -f Release/vtc.so ]; then
	echo "Success!"

	exit 0;
fi

echo "Error!"

exit -1;