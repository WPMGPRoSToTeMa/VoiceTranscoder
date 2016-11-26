@echo off
setlocal EnableDelayedExpansion

set zip="C:\Program Files\7-zip\7z" a

mkdir Publish\addons\VoiceTranscoder\
copy invoker\linux\*.so Publish\addons\VoiceTranscoder\
copy invoker\win32\*.dll Publish\addons\VoiceTranscoder\
copy *.cfg Publish\addons\VoiceTranscoder\

for /F "eol=/ tokens=3,5* delims=;[]	 " %%i in (VoiceTranscoderAPI.h) do (
	if "%%i" == "VOICETRANSCODER_VERSION" (
		set version=%%~j %%k
		set version=!version:;=!
		set version=!version:"=!
		set version=!version: =_!
	)
)

cd Publish
!zip! ..\Invoker\VoiceTranscoder_!version!.zip addons
cd ..

for /F "eol=/ tokens=3,5 delims=;	 " %%i in (VoiceTranscoderAPI.h) do (
	if "%%i" == "VOICETRANSCODER_API_VERSION_MAJOR" (
		set apiVersionMajor=%%j
	) else if "%%i" == "VOICETRANSCODER_API_VERSION_MINOR" (
		set apiVersionMinor=%%j
	)
)

!zip! Invoker\VoiceTranscoder_API_!apiVersionMajor!.!apiVersionMinor!.zip VoiceTranscoderAPI.h