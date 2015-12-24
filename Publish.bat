@echo off
setlocal EnableDelayedExpansion

set zip="C:\Program Files\7-zip\7z" a

mkdir Publish\addons\VoiceTranscoder\
copy invoker\linux\*.so Publish\addons\VoiceTranscoder\
copy invoker\win32\*.dll Publish\addons\VoiceTranscoder\
copy *.cfg Publish\addons\VoiceTranscoder\

for /F "eol=/ tokens=3,4*" %%i in (Main.h) do (
	if "%%i" == "PLUGIN_VERSION[]" (
		set pluginVersion=%%k
		set pluginVersion=!pluginVersion:"=!
		set pluginVersion=!pluginVersion:;=!
		set pluginVersion=!pluginVersion: =_!
	)
)

cd Publish
!zip! ..\Invoker\VoiceTranscoder_!pluginVersion!.zip addons
cd ..

for /F "eol=/ tokens=3,5 delims=;	 " %%i in (VoiceTranscoderAPI.h) do (
	if "%%i" == "VOICETRANSCODER_API_VERSION_MAJOR" (
		set apiVersionMajor=%%j
	) else if "%%i" == "VOICETRANSCODER_API_VERSION_MINOR" (
		set apiVersionMinor=%%j
	)
)

!zip! Invoker\VoiceTranscoder_API_!apiVersionMajor!.!apiVersionMinor!.zip VoiceTranscoderAPI.h