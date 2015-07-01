
#!/bin/sh

# base directory, at top of source tree; replace with absolute path
base=`pwd`
out=outSenstivityTest/master

# configure root dir of interesting stuff
#root=$base/$out/debug/host/linux-x86/product/sim/system
root=$base/$out/target/product/generic/system
export ANDROID_ROOT=$root

# configure bootclasspath
bootpath=$root/framework



#bootpath=/home/salma/Work/android/sourcetree/master/outSenstivityTest/master/target/product/generic/system/framework
export BOOTCLASSPATH=$bootpath/core.jar:$bootpath/ext.jar:$bootpath/framework.jar:$bootpath/android.policy.jar:$bootpath/services.jar
#export BOOTHCLASSPATH=$bootpath/ant-glob.jar:$bootpath/ddmlib-prebuilt.jar:$bootpath/host-libprotobuf-java-2.3.0-lite.jar:$bootpath/org.eclipse.core.commands_3.6.0.I20100512-1500.jar:$bootpath/org-netbeans-api-visual.jar:$bootpath/antlr-runtime.jar:$bootpath/dexdeps.jar:$bootpath/jarjar.jar:$bootpath/org.eclipse.core.expressions_3.4.200.v20100505.jar:$bootpath/org-openide-util.jar:$bootpath/apache-xml-hostdex.jar:$bootpath/doclava.jar:$bootpath/jsilver.jar:$bootpath/org.eclipse.core.runtime_3.6.0.v20100505.jar:$bootpath/propertysheet.jar:$bootpath/asm-tools.jar:$bootpath/dx.jar:$bootpath/kxml2-2.3.0.jar:$bootpath/org.eclipse.equinox.common_3.6.0.v20100503.jar:$bootpath/signapk.jar:$bootpath/bouncycastle-hostdex.jar:$bootpath/easymock.jar:$bootpath/layoutlib_api-prebuilt.jar:$bootpath/org.eclipse.jface_3.6.2.M20110210-1200.jar:$bootpath/swt.jar:$bootpath/bouncycastle-host.jar:$bootpath/guavalib.jar:$bootpath/layoutlib.jar:$bootpath/org.eclipse.osgi_3.6.2.R36x_v20110210.jar:$bootpath/tools-common-prebuilt.jar:$bootpath/conscrypt-hostdex.jar:$bootpath/guava-tools.jar:$bootpath/liblzf.jar:$bootpath/org.eclipse.ui.workbench_3.6.2.M20110210-1200.jar:$bootpath/core-hostdex.jar:$bootpath/hierarchyviewer.jar:$bootpath/okhttp-hostdex.jar:$bootpath/org.eclipse.ui.workbench.texteditor_3.6.1.r361_v20100714-0800.jar

# this is where we create the dalvik-cache directory; make sure it exists
export ANDROID_DATA=/tmp/dalvik_salma
mkdir -p $ANDROID_DATA/dalvik-cache

exec dalvikvm $@
