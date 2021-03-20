@echo off
set GOOGLE_CLOUD_SDK_PATH=PATH\TO\Google Cloud SDK
set GOOGLE_APPLICATION_CREDENTIALS=PUT-YOUR-KEY-HERE.json
set prevdir=%cd%
call "%GOOGLE_CLOUD_SDK_PATH%\cloud_env.bat"
@echo off
cd "%prevdir%"
"%GOOGLE_CLOUD_SDK_PATH%\google-cloud-sdk\platform\bundledpython\python.exe" tts_worker.py %*
