#!/bin/sh

###############################################################################
#                                                                             #
#                             Linux x86 Build                                 #
#                                                                             #
# Builds all gstreamer elements, plugins, etc.                                #
#                                                                             #
###############################################################################

TOP=$(dirname $0)/../../..

#Call common startup routines to load a bunch of variables we'll need
. $TOP/Shared/Scripts/Common.sh
common_startup "Linux" "x86" "Release"

#Load in our own common variables
. $CURR_DIR/Common.sh




#Core
if [ ! -f "$BinDir/libgstreamer-$GstApiVersion.so.0" ]; then 
	mkdir_and_move "$GstIntDir/gstreamer"
	$GstSrcDir/gstreamer/configure --with-package-name='OSSBuild GStreamer' $GstConfigureDefault

	make && make install
	
	arrange_shared "$BinDir" "libgstnet-$GstApiVersion.so" "0" "0.23.0" "libgstnet-$GstApiVersion.la" "gstreamer-net-$GstApiVersion.pc" "$LibDir"
	arrange_shared "$BinDir" "libgstbase-$GstApiVersion.so" "0" "0.23.0" "libgstbase-$GstApiVersion.la" "gstreamer-base-$GstApiVersion.pc" "$LibDir"
	arrange_shared "$BinDir" "libgstcheck-$GstApiVersion.so" "0" "0.23.0" "libgstcheck-$GstApiVersion.la" "gstreamer-check-$GstApiVersion.pc" "$LibDir"
	arrange_shared "$BinDir" "libgstreamer-$GstApiVersion.so" "0" "0.23.0" "libgstreamer-$GstApiVersion.la" "gstreamer-$GstApiVersion.pc" "$LibDir"
	arrange_shared "$BinDir" "libgstcontroller-$GstApiVersion.so" "0" "0.23.0" "libgstcontroller-$GstApiVersion.la" "gstreamer-controller-$GstApiVersion.pc" "$LibDir"
	arrange_shared "$BinDir" "libgstdataprotocol-$GstApiVersion.so" "0" "0.23.0" "libgstdataprotocol-$GstApiVersion.la" "gstreamer-dataprotocol-$GstApiVersion.pc" "$LibDir"
	
	move_files_to_dir "$GstPluginBinDir/*.la" "$GstPluginLibDir/"
	
	cd "$BinDir/"
	rm -f gst-launch && ln -sf gst-launch-$GstApiVersion gst-launch
	rm -f gst-inspect && ln -sf gst-inspect-$GstApiVersion gst-inspect
	rm -f gst-feedback && ln -sf gst-feedback-$GstApiVersion gst-feedback
	rm -f gst-typefind && ln -sf gst-typefind-$GstApiVersion gst-typefind
	rm -f gst-xmllaunch && ln -sf gst-xmllaunch-$GstApiVersion gst-xmllaunch
	rm -f gst-xmlinspect && ln -sf gst-xmlinspect-$GstApiVersion gst-xmlinspect
fi
	
#Base plugins
if [ ! -f "$BinDir/libgstapp-$GstApiVersion.so.0" ]; then 
	mkdir_and_move "$GstIntDir/gst-plugins-base"
	$GstSrcDir/gst-plugins-base/configure --with-package-name='OSSBuild GStreamer Base Plugins' $GstConfigureDefault

	make && make install

	arrange_shared "$BinDir" "libgstapp-$GstApiVersion.so" "0" "0.19.0" "libgstapp-$GstApiVersion.la" "gstreamer-app-$GstApiVersion.pc" "$LibDir"
	arrange_shared "$BinDir" "libgstsdp-$GstApiVersion.so" "0" "0.19.0" "libgstsdp-$GstApiVersion.la" "gstreamer-sdp-$GstApiVersion.pc" "$LibDir"
	arrange_shared "$BinDir" "libgsttag-$GstApiVersion.so" "0" "0.19.0" "libgsttag-$GstApiVersion.la" "gstreamer-tag-$GstApiVersion.pc" "$LibDir"
	arrange_shared "$BinDir" "libgstrtp-$GstApiVersion.so" "0" "0.19.0" "libgstrtp-$GstApiVersion.la" "gstreamer-rtp-$GstApiVersion.pc" "$LibDir"
	arrange_shared "$BinDir" "libgstfft-$GstApiVersion.so" "0" "0.19.0" "libgstfft-$GstApiVersion.la" "gstreamer-fft-$GstApiVersion.pc" "$LibDir"
	arrange_shared "$BinDir" "libgstcdda-$GstApiVersion.so" "0" "0.19.0" "libgstcdda-$GstApiVersion.la" "gstreamer-cdda-$GstApiVersion.pc" "$LibDir"
	arrange_shared "$BinDir" "libgstriff-$GstApiVersion.so" "0" "0.19.0" "libgstriff-$GstApiVersion.la" "gstreamer-riff-$GstApiVersion.pc" "$LibDir"
	arrange_shared "$BinDir" "libgstrtsp-$GstApiVersion.so" "0" "0.19.0" "libgstrtsp-$GstApiVersion.la" "gstreamer-rtsp-$GstApiVersion.pc" "$LibDir"
	arrange_shared "$BinDir" "libgstaudio-$GstApiVersion.so" "0" "0.19.0" "libgstaudio-$GstApiVersion.la" "gstreamer-audio-$GstApiVersion.pc" "$LibDir"
	arrange_shared "$BinDir" "libgstvideo-$GstApiVersion.so" "0" "0.19.0" "libgstvideo-$GstApiVersion.la" "gstreamer-video-$GstApiVersion.pc" "$LibDir"
	arrange_shared "$BinDir" "libgstpbutils-$GstApiVersion.so" "0" "0.19.0" "libgstpbutils-$GstApiVersion.la" "gstreamer-pbutils-$GstApiVersion.pc" "$LibDir"
	arrange_shared "$BinDir" "libgstnetbuffer-$GstApiVersion.so" "0" "0.19.0" "libgstnetbuffer-$GstApiVersion.la" "gstreamer-netbuffer-$GstApiVersion.pc" "$LibDir"
	arrange_shared "$BinDir" "libgstinterfaces-$GstApiVersion.so" "0" "0.19.0" "libgstinterfaces-$GstApiVersion.la" "gstreamer-interfaces-$GstApiVersion.pc" "$LibDir"
	
	move_files_to_dir "$GstPluginBinDir/*.la" "$GstPluginLibDir/"
	
	mv "$BinDir/pkgconfig/gstreamer-floatcast-$GstApiVersion.pc" "$LibDir/pkgconfig/"
	mv "$BinDir/pkgconfig/gstreamer-plugins-base-$GstApiVersion.pc" "$LibDir/pkgconfig/"
	
	cd "$BinDir/"
	ln -sf gst-visualise-$GstApiVersion gst-visualise
fi

#Good plugins
if [ ! -f "$GstPluginBinDir/libgstmatroska.so" -a ! -f "$GstPluginLibDir/libgstmatroska.so" ]; then 
	mkdir_and_move "$GstIntDir/gst-plugins-good"
	$GstSrcDir/gst-plugins-good/configure --with-package-name='OSSBuild GStreamer Good Plugins' $GstConfigureDefault

	make && make install
	
	move_files_to_dir "$GstPluginBinDir/*.la" "$GstPluginLibDir/"
fi

#Ugly plugins
if [ ! -f "$GstPluginBinDir/libgstx264.so" -a ! -f "$GstPluginLibDir/libgstx264.so" ]; then 
	mkdir_and_move "$GstIntDir/gst-plugins-ugly"
	$GstSrcDir/gst-plugins-ugly/configure --with-package-name='OSSBuild GStreamer Ugly Plugins' $GstConfigureDefault

	make && make install
	
	move_files_to_dir "$GstPluginBinDir/*.la" "$GstPluginLibDir/"
fi

#Bad plugins
if [ ! -f "$GstPluginBinDir/libgstsdl.so" -a ! -f "$GstPluginLibDir/libgstsdl.so" ]; then 
	mkdir_and_move "$GstIntDir/gst-plugins-bad"
	
	CFLAGS="$CFLAGS -I$IncludeDir/SDL -I$SharedIncludeDir/SDL"
	$GstSrcDir/gst-plugins-bad/configure --enable-sdl --with-package-name='OSSBuild GStreamer Bad Plugins' $GstConfigureDefault

	make && make install
	
	arrange_shared "$BinDir" "libgstbasevideo-$GstApiVersion.so" "0" "0.0.0" "libgstbasevideo-$GstApiVersion.la" "gstreamer-basevideo-$GstApiVersion.pc" "$LibDir"
	arrange_shared "$BinDir" "libgstphotography-$GstApiVersion.so" "0" "0.0.0" "libgstphotography-$GstApiVersion.la" "gstreamer-photography-$GstApiVersion.pc" "$LibDir"
	arrange_shared "$BinDir" "libgstsignalprocessor-$GstApiVersion.so" "0" "0.0.0" "libgstsignalprocessor-$GstApiVersion.la" "gstreamer-signalprocessor-$GstApiVersion.pc" "$LibDir"
	
	move_files_to_dir "$GstPluginBinDir/*.la" "$GstPluginLibDir/"
	
	cd "$BinDir/"
	mv gst-camera-perf gst-camera-perf-$GstApiVersion
	ln -sf gst-camera-perf-$GstApiVersion gst-camera-perf

	reset_flags
fi

#OpenGL plugins
if [ ! -f "$BinDir/libgstgl-$GstApiVersion.so.0" ]; then 
	mkdir_and_move "$GstIntDir/gst-plugins-gl"
	$GstSrcDir/gst-plugins-gl/configure --with-package-name='OSSBuild GStreamer OpenGL Plugins' $GstConfigureDefault

	make && make install
	
	arrange_shared "$BinDir" "libgstgl-$GstApiVersion.so" "0" "0.0.0" "libgstgl-$GstApiVersion.la" "gstreamer-gl-$GstApiVersion.pc" "$LibDir"
	
	move_files_to_dir "$GstPluginBinDir/*.la" "$GstPluginLibDir/"
fi

#FFmpeg plugins (GPL and LGPL)
for FFmpegSuffix in $LicenseSuffixes
do
	if [ ! -f "$GstPluginBinDir/libgstffmpeg${FFmpegSuffix}.so" -a ! -f "$GstPluginLibDir/libgstffmpeg${FFmpegSuffix}.so" ]; then 
		mkdir_and_move "$GstIntDir/gst-ffmpeg${FFmpegSuffix}"
	
		PKG_CONFIG_PATH="$GstIntDir/gst-ffmpeg${FFmpegSuffix}:$PKG_CONFIG_PATH"
		ln -sf "$SharedLibDir/pkgconfig/libavutil${FFmpegSuffix}.pc" "libavutil.pc"
		ln -sf "$SharedLibDir/pkgconfig/libavcodec${FFmpegSuffix}.pc" "libavcodec.pc"
		ln -sf "$SharedLibDir/pkgconfig/libswscale${FFmpegSuffix}.pc" "libswscale.pc"
		ln -sf "$SharedLibDir/pkgconfig/libavformat${FFmpegSuffix}.pc" "libavformat.pc"
		ln -sf "$SharedLibDir/pkgconfig/libavdevice${FFmpegSuffix}.pc" "libavdevice.pc"
		ln -sf "$SharedLibDir/pkgconfig/libavfilter${FFmpegSuffix}.pc" "libavfilter.pc"
	
		#Patch configure and Makefile.am and regenerate configure
		cd "$GstSrcDir/gst-ffmpeg/"
		patch -u -N -d "$GstSrcDir/gst-ffmpeg/" -i "$GstPatchDir/gst-ffmpeg-configure.ac.patch"
		patch -u -N -d "$GstSrcDir/gst-ffmpeg/ext/" -i "$GstPatchDir/gst-ffmpeg-makefile.am.patch"
		NOCONFIGURE=yes ./autogen.sh

		cd "$GstIntDir/gst-ffmpeg${FFmpegSuffix}"
		$GstSrcDir/gst-ffmpeg/configure --with-system-ffmpeg --with-package-name='OSSBuild GStreamer FFmpeg Plugins' $GstConfigureDefault

		make && make install
	
		move_files_to_dir "$GstPluginBinDir/*.la" "$GstPluginLibDir/"
		rm -f "$GstPluginLibDir/libgstffmpeg${FFmpegSuffix}.la"
		rm -f "$GstPluginLibDir/libgstffmpegscale${FFmpegSuffix}.la"
	
		mv "$GstPluginBinDir/libgstffmpeg.so" "$GstPluginBinDir/libgstffmpeg${FFmpegSuffix}.so"
		mv "$GstPluginBinDir/libgstffmpegscale.so" "$GstPluginBinDir/libgstffmpegscale${FFmpegSuffix}.so"
	
		reset_pkgconfig_path
	fi
done

#GStreamer GNonlin plugins
if [ ! -f "$GstPluginBinDir/libgnl.so" -a ! -f "$GstPluginLibDir/libgnl.so" ]; then 
	mkdir_and_move "$GstIntDir/gnonlin"
	$GstSrcDir/gnonlin/configure --with-package-name='OSSBuild GStreamer Nonlinear Bindings' $GstConfigureDefault

	make && make install

	move_files_to_dir "$GstPluginBinDir/*.la" "$GstPluginLibDir/"
fi

#GStreamer Python bindings
if [ ! -f "$LibDir/python2.6/site-packages/gst-$GstApiVersion/gst/_gst.so" ]; then 
	mkdir_and_move "$GstIntDir/gst-python"
	$GstSrcDir/gst-python/configure --with-package-name='OSSBuild GStreamer Python Bindings' $GstConfigureDefault
	
	make && make install
	
	move_files_to_dir "$GstPluginBinDir/*.la" "$GstPluginLibDir/"
	
	remove_files_from_dir "$LibDir/python2.6/site-packages/*.la"
	remove_files_from_dir "$LibDir/python2.6/site-packages/gst-$GstApiVersion/gst/*.la"
	remove_files_from_dir "$LibDir/python2.6/site-packages/gst-$GstApiVersion/gst/*.pyc"
	remove_files_from_dir "$LibDir/python2.6/site-packages/gst-$GstApiVersion/gst/*.pyo"
	remove_files_from_dir "$LibDir/python2.6/site-packages/gst-$GstApiVersion/gst/extend/*.pyc"
	remove_files_from_dir "$LibDir/python2.6/site-packages/gst-$GstApiVersion/gst/extend/*.pyo"
	
	mv "$BinDir/pkgconfig/gst-python-$GstApiVersion.pc" "$LibDir/pkgconfig/"
fi

#Farsight2
if [ ! -f "$BinDir/libgstfarsight-$GstApiVersion.so.0" ]; then 
	mkdir_and_move "$GstIntDir/farsight2"
	$GstSrcDir/farsight2/configure --with-package-name='OSSBuild Farsight2' $GstConfigureDefault

	make && make install

	arrange_shared "$BinDir" "libgstfarsight-$GstApiVersion.so" "0" "0.3.1" "libgstfarsight-$GstApiVersion.la" "farsight2-$GstApiVersion.pc" "$LibDir"

	remove_files_from_dir "$LibDir/python2.6/site-packages/farsight.la"
	remove_files_from_dir "$Farsight2BinDir/*.la"
	move_files_to_dir "$Farsight2BinDir/*.so" "$Farsight2LibDir/"
	move_files_to_dir "$GstPluginBinDir/*.la" "$GstPluginLibDir/"
fi

#GStreamer Farsight2 plugins
if [ ! -f "$GstPluginBinDir/libgstnetsim.so" -a ! -f "$GstPluginLibDir/libgstnetsim.so" ]; then 
	mkdir_and_move "$GstIntDir/gst-plugins-farsight"
	$GstSrcDir/gst-plugins-farsight/configure --with-package-name='OSSBuild GStreamer Farsight2 Plugins' $GstConfigureDefault
	
	make && make install
	
	move_files_to_dir "$GstPluginBinDir/*.la" "$GstPluginLibDir/"
fi

#GStreamer Sharp (.NET) bindings
if [ ! -f "$BinDir/libgstreamersharpglue-$GstApiVersion.so" ]; then 
	mkdir_and_move "$GstIntDir/gstreamer-sharp"
	
	cd "$GstSrcDir/gstreamer-sharp/"
	NOCONFIGURE=yes ./autogen.sh

	cd "$GstIntDir/gstreamer-sharp/"
	$GstSrcDir/gstreamer-sharp/configure --with-package-name='OSSBuild GStreamer-Sharp .NET Bindings' $GstConfigureDefault
	
	#Technically this is not the GStreamer API version. At the time of writing, it was 0.9.2
	sed -e "s:@API_VERSION@:$GstApiVersion:g" "$GstSrcDir/gstreamer-sharp/gstreamer-sharp/AssemblyInfo.cs.in" > "$GstSrcDir/gstreamer-sharp/gstreamer-sharp/AssemblyInfo.cs"
	
	make && make install

	move_files_to_dir "$BinDir/pkgconfig/gstreamer-sharp-$GstApiVersion.pc" "$LibDir/pkgconfig/"
	move_files_to_dir "$BinDir/libgstreamersharpglue-$GstApiVersion.la" "$LibDir/"
	
	mv "$BinDir/mono" "$LibDir/"
fi

#Move plugins to the lib directory
move_files_to_dir "$GstPluginBinDir/*" "$GstPluginLibDir/"

#Get rid of files/directories we don't need
rm -rf "$BinDir/pkgconfig/"
rm -rf "$Farsight2BinDir/"
rm -rf "$GstPluginBinDir/"
remove_files_from_dir "$GstPluginLibDir/*.la"


#Merge everything into Shared/

#Shared/share/
mkdir -p "$SharedShareDir/gapi/"
mkdir -p "$SharedShareDir/locale/"
mkdir -p "$SharedShareDir/aclocal/"
mkdir -p "$SharedShareDir/gst-python/"
mkdir -p "$SharedShareDir/gstreamer-$GstApiVersion/"

cd "$ShareDir/gapi/" && cp -rupd * "$SharedShareDir/gapi/"
cd "$ShareDir/locale/" && cp -rupd * "$SharedShareDir/locale/"
cd "$ShareDir/aclocal/" && cp -rupd * "$SharedShareDir/aclocal/"
cd "$ShareDir/gst-python/" && cp -rupd * "$SharedShareDir/gst-python/"
cd "$ShareDir/gstreamer-$GstApiVersion/" && cp -rupd * "$SharedShareDir/gstreamer-$GstApiVersion/"

#Shared/bin/
cd "$BinDir" && copy_files_to_dir "*" "$SharedBinDir"

#Shared/include/
cd "$IncludeDir" && cp -rupd * "$SharedIncludeDir"

#Shared/etc/
mkdir -p "$SharedEtcDir/gconf"
cd "$EtcDir/gconf/" && cp -rupd * "$SharedEtcDir/gconf"

#Shared/lib/
mkdir -p "$SharedLibDir/mono"
mkdir -p "$SharedLibDir/python2.6"
mkdir -p "$SharedLibDir/gstreamer-$GstApiVersion"
mkdir -p "$SharedLibDir/farsight2-$Farsight2ApiVersion"
cd "$LibDir" 
cp -rupd "mono" "$SharedLibDir"
cp -rupd "python2.6" "$SharedLibDir"
cp -rupd "gstreamer-$GstApiVersion" "$SharedLibDir"
cp -rupd "farsight2-$Farsight2ApiVersion" "$SharedLibDir"

#Create pkgconfig/libtool templates
create_templates




#Call common shutdown routines
common_shutdown

