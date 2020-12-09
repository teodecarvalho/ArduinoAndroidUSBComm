
buildozer init
cp ./usb4a/intent-filter.xml .
sed -i  '/Pattern to whitelist for the whole project/ { n; s/.*/android.whitelist = lib-dynload\/termios.so/; }' buildozer.spec
sed -i  '/^requirements = / { s/.*/requirements = kivy, pyjnius, usb4a, pyserial/; }' buildozer.spec
sed -i  '/# (str) XML file to include as an intent filters in/ { n; s/.*/android.manifest.intent_filters = intent-filter.xml /; }' buildozer.spec
sed -i '/title = My Application/ {s/My Application/Greenhouse/;}' buildozer.spec
sed -i '/package.name = myapp/ {s/myapp/greenhouse/;}' buildozer.spec
buildozer android debug deploy run





