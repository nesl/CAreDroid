#http://source.android.com/source/building-running.html
source build/envsetup.sh
#export TARGET_BUILD_TYPE=debug
#export OUT_DIR_COMMON_BASE=/home/salma/Work/android/sourcetree/master/out
export OUT_DIR_COMMON_BASE=/home/salma/Work/android/orgsourcetree/master/outMaguro
lunch full_maguro-eng
make -j4
