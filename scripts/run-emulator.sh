#run the emulator with the name of the required avd 
# avd is located in ~/.android/avd 


# for debug 
# export ANDROID_LOG_TAGS='*:v jdwp:v dalvikvm:v dalvikvmi:v'

 #emulator -avd ClassLoaderTestSource

export ANDROID_LOG_TAGS=ANDROID_LOG_DEBUG

emulator -avd HiddenDynamicLoading -kernel /home/salma/Work/android/orgsourcetree/master/out/host/linux-x86/sdk/android-sdk_eng.salma_linux-x86/system-images/android-4.2.2.2.2.2.2.2.2.2/armeabi-v7a/kernel-qemu -system /home/salma/Work/android/orgsourcetree/master/out/host/linux-x86/sdk/android-sdk_eng.salma_linux-x86/system-images/android-4.2.2.2.2.2.2.2.2.2/armeabi-v7a/system.img -data /home/salma/Work/android/orgsourcetree/master/out/host/linux-x86/sdk/android-sdk_eng.salma_linux-x86/system-images/android-4.2.2.2.2.2.2.2.2.2/armeabi-v7a/userdata.img -ramdisk /home/salma/Work/android/orgsourcetree/master/out/host/linux-x86/sdk/android-sdk_eng.salma_linux-x86/system-images/android-4.2.2.2.2.2.2.2.2.2/armeabi-v7a/ramdisk.img -wipe-data
