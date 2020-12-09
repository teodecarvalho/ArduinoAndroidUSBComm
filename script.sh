#!/bin/bash
yes | cp -rf /media/psf/Home/Downloads/Arduino/GreenhouseVeg/* .
buildozer android debug deploy run
adb logcat | grep python
