# For emulator
adb shell setprop dalvik.vm.execution-mode int:portable
#adb shell setprop dalvik.vm.checkjni false
adb shell stop
adb shell start 

# For a device.
#The fast interpreter is enabled by default. On platforms without native support, you may want to switch to the portable interpreter.
#This can be controlled with the dalvik.vm.execution-mode system property. 
#adb shell "echo dalvik.vm.execution-mode = int:portable >> /data/local.prop"
#adb reboot 


# note: 
# If you receive this error message 
# error: device offline
# error: device offline
# error: device offline

# Wait a moment and try agai because a device (emulator) must be launched first

