#!/bin/bash
yes | cp -rf /media/psf/Home/Downloads/ArduinoAndroidUSBComm/* .
buildozer android debug deploy run
adb logcat | grep python
