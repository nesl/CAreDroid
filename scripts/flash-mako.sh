#inside the out folder of the mako under product 
export ANDROID_PRODUCT_OUT=~/Work/android/orgsourcetree/master/outMako/master/target/product/mako/
export ANDROID_PRODUCT_OUT=/home/salma/Work/CAreDroid/orgsourcetree/master/outMako/master/target/product/mako/
adb reboot bootloader
fastboot -w flashall 


