@echo off
setlocal EnableDelayedExpansion

mkdir Publish\addons\VoiceTranscoder\
copy Invoker\* Publish\addons\VoiceTranscoder\
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
"C:\Program Files\7-zip\7z" a ..\Invoker\VoiceTranscoder_!pluginVersion!.zip addons
cd ..