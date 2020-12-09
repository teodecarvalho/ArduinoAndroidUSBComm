cp -r ./usb4a/xml ./.buildozer/android/platform/build*/dists/*/src/main/res/
find ./ -name AndroidManifest.tmpl.xml | grep greenhouse | xargs sed -i '/uses-feature/a \    <uses-feature android:name="android.hardware.usb.host" \/\>'
