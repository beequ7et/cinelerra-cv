AC_INIT(cinelerra-cv, 2.2)

AC_CANONICAL_SYSTEM

# Test for minimal version of autoconf
AC_PREREQ(2.57)

AM_INIT_AUTOMAKE([foreign subdir-objects])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
AC_CONFIG_HEADER(config.h)
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.12.1)
AC_CONFIG_MACRO_DIR([m4])
AM_PROG_AS

AC_LANG_CPLUSPLUS
AC_LANG_C
AC_PROG_CXX

dnl
dnl GREP_CXXFLAGS(flag, cmd_if_missing, cmd_if_present)
dnl
AC_DEFUN([GREP_CXXFLAGS],
[case "$CXXFLAGS" in
"$1" | "$1 "* | *" $1" | *" $1 "* )
  ifelse($#, 3, [$3], [:])
  ;;
*)
  $2
  ;;
esac
])
dnl
dnl GREP_CFLAGS(flag, cmd_if_missing, cmd_if_present)
dnl
AC_DEFUN([GREP_CFLAGS],
[case "$CFLAGS" in
"$1" | "$1 "* | *" $1" | *" $1 "* )
  ifelse($#, 3, [$3], [:])
  ;;
*)
  $2
  ;;
esac
])

if eval "test x$GCC = xyes"; then
  for flag in \
      -Wformat \
      -Werror=format-security
  do
    GREP_CXXFLAGS($flag, [ CXXFLAGS="$CXXFLAGS $flag" ])
  done
  for flag in \
      -Wformat \
      -Werror=format-security
  do
    GREP_CFLAGS($flag, [ CFLAGS="$CFLAGS $flag" ])
  done
  AC_ARG_WITH([lots-of-warnings],
	AC_HELP_STRING([--with-lots-of-warnings], [ask the compiler to enable lots of warning flags]),[
  for flag in \
      -Wall \
      -Werror=format-security \
      -Wextra \
      -Wcast-qual \
      -Wpointer-arith \
      -Wreturn-type \
      -Wunused \
      -pedantic
  do
    GREP_CXXFLAGS($flag, [ CXXFLAGS="$CXXFLAGS $flag" ])
  done
  for flag in \
      -Wall \
      -Werror=format-security \
      -Wextra \
      -Wcast-qual \
      -Wpointer-arith \
      -Wreturn-type \
      -Wunused \
      -pedantic
  do
    GREP_CFLAGS($flag, [ CFLAGS="$CFLAGS $flag" ])
  done
])
fi

AC_ENABLE_SHARED
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

# hack for libtool 1.5 -- this is ugly and probably wrong, but works
LTC_FLAGS="--tag=CC"
LTCXX_FLAGS="--tag=CXX"
AC_SUBST(LTC_FLAGS)
AC_SUBST(LTCXX_FLAGS)

dnl Check for nasm
AC_CHECK_PROG(HAVE_NASM, nasm, yes)
if test x$HAVE_NASM != "xyes"; then
    AC_MSG_ERROR(nasm is missing. Please install it.)
fi

# hack for using nasm for .S files
CCAS="\$(top_srcdir)/admin/nasm"
AC_SUBST(CCAS)
AC_SUBST(CCASFLAGS)

dnl Check for Video4Linux2
AH_TEMPLATE(HAVE_VIDEO4LINUX2, [Define to 1 if Video4Linux2 is available.])
AC_CHECK_HEADER([linux/videodev2.h], 
	[AC_CHECK_MEMBER(struct v4l2_buffer.index, [AC_DEFINE(HAVE_VIDEO4LINUX2)], [],
		[#include <linux/videodev2.h>])],
	[],[] )

############## SPECIAL DIRECTORIES
AC_ARG_WITH(plugindir,
						AC_HELP_STRING([--with-plugindir], [sets the directory where the plugins should be installed]),
						[ plugindir=$withval ],
						[ plugindir="\${exec_prefix}/lib/cinelerra" ])
AC_SUBST(plugindir)

AC_ARG_WITH(fontsdir,
						AC_HELP_STRING([--with-fontsdir], [sets the directory where the fonts should be installed]),
						[ fontsdir=$withval ],
						[ fontsdir="\${plugindir}/fonts" ])
AC_SUBST(fontsdir)
############## END OF SPECIAL DIRECTORIES

############## XLIB
AC_PATH_XTRA
if test "$no_x" = yes; then
  if test "x$have_x" = "xdisabled"; then
    AC_MSG_WARN([Do not use --without-x or --with-x=no.])
  else
    AC_MSG_WARN([X Windows development tools were not found.])
    AC_MSG_WARN([Please install xlib-dev or xorg-x11-devel.])
  fi
  AC_MSG_ERROR([Cinelerra requires X Windows.])
fi
AH_TEMPLATE(HAVE_LIBXXF86VM, [Define to 1 if libXxf86vm is available.])
AC_DEFINE(HAVE_LIBXXF86VM)
############# XFT
AC_MSG_CHECKING([whether to use xft])
AC_ARG_ENABLE(xft, AC_HELP_STRING([--enable-xft],
	[use freetype/fontconfig/xft (default: on)]),
	[enable_xft=$enableval], [enable_xft="default"])
XFT_CFLAGS=""
XFT_LIBS=""
if test "$enable_xft" = "no" ; then
	AC_MSG_RESULT([$enable_xft])
	XFT_CFLAGS=`pkg-config xft --cflags 2>/dev/null` || found_xft="no"
	XFT_LIBS=`pkg-config xft --libs 2>/dev/null` || found_xft="no"
	CXXFLAGS="$XFT_CFLAGS $CXXFLAGS"
else
	found_xft="yes"
	dnl make sure package configurator (pkg-config xft or pkg-config
	dnl says that xft is present.
	XFT_CFLAGS=`pkg-config xft --cflags 2>/dev/null` || found_xft="no"
	XFT_LIBS=`pkg-config xft --libs 2>/dev/null` || found_xft="no"
	if ! test "$found_xft" = "no" ; then
		found_xft=yes
	fi
	AC_MSG_RESULT([$found_xft])
	dnl print a warning if xft is unusable and was specifically requested
	if test "$found_xft" = "no" ; then
		if test "$enable_xft" = "yes" ; then
			AC_MSG_WARN([Can't find xft configuration, or xft is unusable])
		fi
		enable_xft=no
		XFT_CFLAGS=""
		XFT_LIBS=""
	else
		enable_xft=yes
		## To force to all project - uncomment
		##X_EXTRA_LIBS="$XFT_LIBS $X_EXTRA_LIBS"
		CXXFLAGS="$XFT_CFLAGS $CXXFLAGS"
		AH_TEMPLATE(HAVE_XFT, [Define to 1 if libXft is available.])
		AC_DEFINE(HAVE_XFT)
		## need some fix to compile with XFT
		AC_DEFINE(
			[PNG_SETJMP_NOT_SUPPORTED], [1],
			[Workaround to fix compile with xft])
	fi
fi
AC_SUBST(XFT_CFLAGS)
AC_SUBST(XFT_LIBS)
############## END XLIB

############## SOUND
# the sound options are propagated by SOUND_CFLAGS and SOUND_LDFLAGS
AC_ARG_ENABLE(oss, 
							AC_HELP_STRING([--disable-oss], [disable support for oss (default=enabled)]))
AC_ARG_ENABLE(alsa, 
	AC_HELP_STRING([--disable-alsa],[disable support for ALSA (default=autodetect)]),
	[enable_alsa=$enableval],[enable_alsa=yes])
AC_ARG_ENABLE(esd, 
	AC_HELP_STRING([--disable-esd], [disable support for ESD (default=autodetect)]),
	[enable_esd=$enableval],[enable_esd=yes])

if test "x$enable_oss" = "xyes" || test "x$enable_oss" = "x"; then
	SOUND_CFLAGS="-DHAVE_OSS $SOUND_CFLAGS"
fi
if test "x$enable_alsa" = "xyes"; then
	AM_PATH_ALSA(1.0.2,[alsa=yes])
	if test "x$alsa" = "xyes"; then
		SOUND_CFLAGS="$ALSA_CFLAGS -DHAVE_ALSA $SOUND_CFLAGS"
		SOUND_LDFLAGS="$ALSA_LIBS $SOUND_LDFLAGS"
		enable_alsa=ok
	fi
	dnl This added -lasound to LIBS. LIBS is used in every link command.
	dnl But we do not want to link all shared libraries against -lasound.
	dnl So we remove it again.
	LIBS=`echo "$LIBS" | sed -e s/-lasound//g`
fi

if test "x$enable_esd" = "xyes"; then
	AM_PATH_ESD(,[esound=yes])
	if test "x$esound" = xyes; then
		SOUND_CFLAGS="$ESD_CFLAGS -DHAVE_ESOUND $SOUND_CFLAGS"
		SOUND_LDFLAGS="$ESD_LIBS $SOUND_LDFLAGS"
		enable_esd=ok
	fi
fi
AC_SUBST(SOUND_CFLAGS)
AC_SUBST(SOUND_LDFLAGS)
############### END OF SOUND

############## MJPEGTOOLS (for YUV4MPEG)
PKG_CHECK_MODULES(MJPEG, mjpegtools,[mjpegtools=yes],:)
AC_SUBST(MJPEG_CFLAGS)
AC_SUBST(MJPEG_LIBS)
AC_MSG_CHECKING([for number of arguments to y4m_write_frame_header])
# check if mjpegtools has 3 arguements for y4m_write_frame_header
# Note: this test came from transcode...
save_CFLAGS="$CFLAGS"
save_LIBS="$LIBS"
CFLAGS="$CFLAGS $MJPEG_CFLAGS"
LIBS="$LIBS $MJPEG_LIBS"
AC_TRY_LINK(
[
#include <mjpegtools/yuv4mpeg.h>
#include <mjpegtools/mpegconsts.h>
],
[y4m_write_frame_header(1,NULL,NULL)],
[
AC_DEFINE(
	   [MJPEGTOOLS_Y4M_WRITE_FRAME_HEADER__3ARGS], 
           [1], 
           [using y4m_write_frame_header with 3 arguments]
          )
mjpeg_version_info="3 arguments (new versions)"
],[mjpeg_version_info="assuming 2 arguments (older versions)"])
AC_MSG_RESULT($mjpeg_version_info)
CFLAGS="$save_CFLAGS"
LIBS="$save_LIBS"
############## END MJPEGTOOLS

############## MISC LIBRARIES
AC_CHECK_HEADER(uuid/uuid.h,[libuuidh=yes])
AC_CHECK_LIB(uuid, uuid_clear,[libuuid=yes])

AC_CHECK_HEADER(fftw3.h,[libfftw3h=yes])
AC_CHECK_LIB(fftw3, fftw_free,[libfftw3=yes])

PKG_CHECK_MODULES(LIBDV,libdv >= 0.103,[libdv=yes],:)
AC_SUBST(LIBDV_CFLAGS)
AC_SUBST(LIBDV_LIBS)

PKG_CHECK_MODULES(PNG,libpng,[libpng=yes],:)

AC_CHECK_HEADER(jpeglib.h,[libjpegh=yes])
AC_CHECK_LIB(jpeg,jpeg_start_decompress,[libjpeg=yes])

AC_CHECK_HEADER(tiffio.h,[libtiffh=yes])
AC_CHECK_LIB(tiff,TIFFOpen,[libtiff=yes])

AC_ARG_ENABLE(freetype2,
	AC_HELP_STRING([--enable-freetype2],[enables freetype2 usage (default is use pkg-config detection)]),
	[ 
	  [freetype2=yes] ],
	[ PKG_CHECK_MODULES(FREETYPE,freetype2,[freetype2=yes],:) ])

# OpenCV is used by the findobj plugin.  Check if the needed libraries
# exist.  The functions to look for are picked by random from the
# libraries, and might not be the exact ones the findobj plugin need.
AC_CHECK_HEADER(opencv2/legacy/legacy.hpp)
AC_CHECK_HEADER(opencv2/legacy/compat.hpp)
AC_CHECK_HEADER(opencv2/core/core_c.h, [opencvh=yes])
AC_CHECK_LIB(opencv_legacy, cvChangeDetection)
AC_CHECK_LIB(opencv_objdetect, cvLoadLatentSvmDetector)
AC_CHECK_LIB(opencv_video, cvCamShift)
AC_CHECK_LIB(opencv_core, cvReleaseMemStorage, [opencv=yes])
AC_MSG_CHECKING(for SURF support in opencv)
AC_TRY_LINK([#include "opencv2/legacy/legacy.hpp"
#include "opencv2/legacy/compat.hpp"],
[try  {
cvExtractSURF(object_image, 
	0, 
	&object_keypoints, 
	&object_descriptors, 
	storage, 
	params,
	0);
} catch (cv::Exception e) {
  return 1;
}
return 0;],
	[HAVE_OPENCV_SURF=yes],[HAVE_OPENCV_SURF=no])
AC_MSG_RESULT([$HAVE_OPENCV_SURF])
if test yes = "$HAVE_OPENCV_SURF" ; then
	HAVE_OPENCV_SURF=1
else
	HAVE_OPENCV_SURF=0
fi
AC_DEFINE_UNQUOTED(HAVE_OPENCV_SURF, $HAVE_OPENCV_SURF,
	[define if SURF support is compiled into opencv])
if test "x$opencv" = "xyes" && test "x$opencvh" = "xyes" ; then
	enable_findobject=ok
fi
AM_CONDITIONAL(ENABLE_FINDOBJECT,test "x$enable_findobject" = "xok")
############## END OF MISC LIBRARIES

LARGEFILE_CFLAGS="-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64"
AC_SUBST(LARGEFILE_CFLAGS)

############## A52DEC
AC_CHECK_LIB(a52,a52_init,[liba52=yes])
AC_CHECK_HEADER(a52dec/a52.h,[liba52h=yes])

if test "x$liba52$liba52h" = "xyesyes"; then
    A52DEC_CFLAGS=
    A52DEC_LIBS=-la52
fi
AC_SUBST(A52DEC_CFLAGS)
AC_SUBST(A52DEC_LDFLAGS)
############## END OF A52DEC

############## LIBSNDFILE
AC_CHECK_LIB(sndfile, sf_open,[libsndfile=yes])
AC_CHECK_HEADERS(sndfile.h,[libsndfileh=yes])
############## END OF LIBSNDFILE

############## OpenEXR
PKG_CHECK_MODULES(OPENEXR, OpenEXR,[libOpenEXR=yes],:)
AC_SUBST(OPENEXR_CFLAGS)
AC_SUBST(OPENEXR_LIBS)
############## END OF OpenEXR

############## LIBFAAD, LIBFAAC
AC_CHECK_LIB(faac, faacEncOpen,[libfaac=yes])
AC_CHECK_HEADER(faac.h,[libfaach=yes])

AC_CHECK_LIB(faad, faacDecInit,[libfaad=yes],
	AC_CHECK_LIB(faad, NeAACDecInit,[libfaad=yes]))
AC_CHECK_HEADER(faad.h,[libfaadh=yes])
if test "x$libfaad$libfaadh" = xyesyes; then
    FAAD_CFLAGS=
    FAAD_LIBS="-lfaad"
fi
AC_SUBST(FAAD_CFLAGS)
AC_SUBST(FAAD_LIBS)
############## END OF LIBFAAD, LIBFAAC

############## FIREWIRE
# firewire settings are propagated through FIREWIRE_CFLAGS and FIREWIRE_LDFLAGS
AC_ARG_ENABLE(firewire, 
							AC_HELP_STRING([--disable-firewire], [disable support for firewire (default=enabled)]),
							[ enable_firewire=$enableval ],
							[ enable_firewire=yes ])

if test "x$enable_firewire" = "xyes"; then
	PKG_CHECK_MODULES(LIBRAW1394, libraw1394 >= 1.2.0,[libraw1394=yes],:)
	PKG_CHECK_MODULES(LIBIEC61883, libiec61883,[libiec61883=yes],:)

	AC_CHECK_LIB(avc1394, avc1394_send_command,[libavc1394=yes],:,$LIBRAW1394_LIBS)
	AC_CHECK_HEADER([libavc1394/avc1394.h],[libavc1394h=yes])
	AC_CHECK_LIB(rom1394, rom1394_get_bus_id,[librom1394=yes],:,$LIBRAW1394_LIBS)
	AC_CHECK_HEADER([libavc1394/rom1394.h],[librom1394h=yes])
fi

if test "x$enable_firewire$libraw1394$libiec61883$libavc1394$libavc1394h$librom1394$librom1394h" = "xyesyesyesyesyesyesyes"; then
	FIREWIRE_CFLAGS="-DHAVE_FIREWIRE $LIBIEC61883_CFLAGS $LIBRAW1394_CFLAGS"
	FIREWIRE_LDFLAGS="-lavc1394 -lrom1394 $LIBIEC61883_LIBS $LIBRAW1394_LIBS"
	enable_firewire=ok
fi
AC_SUBST(FIREWIRE_CFLAGS)
AC_SUBST(FIREWIRE_LDFLAGS)
AM_CONDITIONAL(HAVE_FIREWIRE,test "x$enable_firewire" = "xok")
############## END OF FIREWIRE

############# BUILDINFO display, (for displaying version / date)
AC_ARG_WITH(buildinfo,
          AC_HELP_STRING(
	  	  [--with-buildinfo],
		  [includes revision information in the 'About' box; allowed values: svn, git; add /recompile to force rebuild on each make invocation, or cust/"Custom string" for packagers]),
	[ buildinfo=$withval],
	[ buildinfo="none" ])
AC_SUBST(buildinfo)

BUILDINFO_repository=$(echo $buildinfo | sed 's=/recompile==')
BUILDINFO_recompile=$(echo $buildinfo | sed 's=.*/recompile=yes=')
BUILDINFO_custom=$(echo $buildinfo | sed 's=cust.*=yes=')
BUILDINFO_CUSTOM_TAG=$(echo $buildinfo | sed 's=cust/==')
AC_SUBST(BUILDINFO_CUSTOM_TAG)
AM_CONDITIONAL(BUILDINFO_GIT, test "x$BUILDINFO_repository" = "xgit")
AM_CONDITIONAL(BUILDINFO_SVN, test "x$BUILDINFO_repository" = "xsvn")
AM_CONDITIONAL(BUILDINFO_RECOMPILE, test "x$BUILDINFO_recompile" = "xyes")
AM_CONDITIONAL(BUILDINFO_CUST, test "x$BUILDINFO_custom" = "xyes")

echo removing cinelerra/versioninfo.h to force a re-build
rm -f cinelerra/versioninfo.h > /dev/null 2>&1

############# END BUILDINFO display, (for displaying version / date)

############# CSS SUPPORT IN LIBMPEG3
if test "x$enable_css" = "xyes"; then
	CSS_CFLAGS="-DHAVE_CSS"
fi
AC_SUBST(CSS_CFLAGS)
AC_ARG_ENABLE(css, 
							AC_HELP_STRING([--disable-css], [disable support for css in libmpeg3 (default=enabled)]),,
							[ enable_css=$enableval ],
							[ enable_css=yes ])
############## END OF CSS SUPPORT IN LIBMPEG3


############## libx264
LIBX264_LIBS=""
PKG_CHECK_MODULES([LIBX264], [x264], [libx264=yes], [libx264=no])
AC_CHECK_HEADER(x264.h, [libx264h=yes])
AC_SUBST(LIBX264_LIBS)
##############

############## MMX / 3DNOW  POWERPC / ALTIVEC

AC_ARG_ENABLE(mmx, 
	AC_HELP_STRING([--enable-mmx], [enables support for mmx (default is autodetect)]),
		[ enable_mmx=$enableval ],[ enable_mmx=auto ])
AC_ARG_ENABLE(3dnow, 
	AC_HELP_STRING([--enable-3dnow], [enables support for 3dnow (default is disabled)]),
		[ enable_3dnow=$enableval ], [ enable_3dnow=no ])

AC_ARG_ENABLE(altivec, 
	AC_HELP_STRING([--enable-altivec], [enables altivec support (default is disabled)]),
		[ enable_altivec=$enableval ],
		[ enable_altivec=no ])

case "$target_cpu" in
i586 | i686)
	test "x$enable_mmx" = "xauto" && enable_mmx=yes

	CPU_CFLAGS="-DX86_CPU $CPU_CFLAGS"

	ffmpeg_32bit=yes

	if test "x$enable_mmx" = "xyes"; then
		CPU_CFLAGS="-DHAVE_MMX -DUSE_MMX $CPU_CFLAGS" # -D_MMX_ doesn't work
		enable_mmx32=yes
	fi
	if test "x$enable_3dnow" = "xyes"; then
		CPU_CFLAGS="-DHAVE_3Dnow $CPU_CFLAGS" # -DUSE_3DNOW --> don't use, not compilin
	fi
;;
x86_64)
	test "x$enable_mmx" = "xauto" && enable_mmx=yes
	CPU_CFLAGS="-DX86_CPU $CPU_CFLAGS"
	if test "x$enable_mmx" = "xyes"; then
		CPU_CFLAGS="-DHAVE_MMX -DUSE_MMX $CPU_CFLAGS"

        fi
;;
powerpc)
	CPU_CFLAGS="-mcpu=powerpc $CPU_CFLAGS"
	if test "x$enable_altivec" = "xyes"; then
		CPU_CFLAGS="-maltivec -mabi=altivec $CPU_CFLAGS"
	fi
;;
esac

test "x$enable_mmx" = "xauto" && enable_mmx=no

AM_CONDITIONAL(USEMMX, test "x$enable_mmx" = "xyes")
AM_CONDITIONAL(USEMMX32, test "x$enable_mmx32" = "xyes")
AM_CONDITIONAL(TARGET_BUILTIN_VECTOR,test "x$enable_mmx" = "xyes")
AM_CONDITIONAL(USE3DNOW, test "x$enable_3dnow" = "xyes")
AM_CONDITIONAL(TARGET_ARCH_POWERPC, test "x$target_cpu" = "xpowerpc")
AM_CONDITIONAL(TARGET_ALTIVEC, test "x$enable_altivec" = "xyes")
AM_CONDITIONAL(FFMPEG_ARCH_32BIT, test "x$ffmpeg_32bit" = "xyes")
AC_SUBST(LIBDECORE_LIBADD)
AC_SUBST(CPU_CFLAGS)

############## END OF MMX / 3DNOW

############ external ffmpeg
AC_ARG_WITH([external-ffmpeg], AC_HELP_STRING([--with-external-ffmpeg], [use external ffmpeg library]))

if test "x$with_external_ffmpeg" = "xyes"; then
     PKG_CHECK_MODULES([FFMPEG_TEMP], [libavcodec libpostproc])
     FFMPEG_FOLDER=""
     FFMPEG_EXTERNALTEXT="External ffmpeg"
		   
     dnl --------------------------------------------------------------
     dnl check if libavcodec contains img_convert
     dnl if not, that means that libswscale is compiled in
     
     AC_MSG_CHECKING(for ffmpeg swscale support)
     saved_LIBS="$LIBS"
     LIBS="$saved_LIBS $FFMPEG_TEMP_LIBS"
     AC_TRY_LINK([#include <libavcodec/avcodec.h>],
		 [img_convert(0, 0, 0,0,0,0)],
		 enable_ffmpeg_swscale=no,enable_ffmpeg_swscale=yes)
     LIBS="$saved_LIBS"
     AC_MSG_RESULT($enable_ffmpeg_swscale)
     if test x"$enable_ffmpeg_swscale" = xyes; then
	     dnl AC_DEFINE(HAVE_SWSCALER)
	     PKG_CHECK_MODULES([FFMPEG], [libavcodec libpostproc libswscale])
	     FFMPEG_CFLAGS="$FFMPEG_CFLAGS -I/usr/include/libavcodec -I/usr/include/libswscale -DHAVE_SWSCALER"
     else
	     PKG_CHECK_MODULES([FFMPEG], [libavcodec libpostproc])
     fi
     FFMPEG_EXTERNALTEXT="External ffmpeg"
else
     FFMPEG_FOLDER=ffmpeg
     FFMPEG_CFLAGS="-I\$(top_srcdir)/quicktime/ffmpeg"
     FFMPEG_LIBS="-L\$(top_builddir)/quicktime/ffmpeg/libavcodec -lavcodec-cinelerra \
		-L\$(top_builddir)/quicktime/ffmpeg/libavutil -lavutil-cinelerra"
     FFMPEG_EXTERNALTEXT="Internal ffmpeg"
fi

AC_SUBST(FFMPEG_FOLDER)
AC_SUBST(FFMPEG_CFLAGS)
AC_SUBST(FFMPEG_LIBS)
AC_SUBST(FFMPEG_EXTERNALTEXT)
############# END external ffmpeg

############# run ffmpeg configure to generates avconfig.h
echo --- FFMPEG CONFIGURATION SECTION --------------
if test "x$with_external_ffmpeg" = "xyes"; then
	echo Using external ffmpeg - no configuration needed
else
	ff_conf_args="--disable-ffmpeg --disable-ffserver --disable-ffplay \
		--disable-static --enable-shared --build-suffix=-cinelerra"

	if test "x$enable_mmx" = "xno"; then
		ff_conf_args="$ff_conf_args --disable-mmx"
	fi

	# Where altivec is not present
	if test "x$enable_altivec" = "xno"; then
		ff_conf_args="$ff_conf_args --disable-altivec"
	fi


	srcdir_c=$(readlink -f $srcdir) &&
	mkdir -p quicktime/ffmpeg/ &&
	(
	cd quicktime/ffmpeg/
	# if --libdir was not set, then it expands to '$exec_prefix/lib'
	# if --exec-prefix was not set, then it expands to NONE
	# this makes sure $libdir is expanded to a useful value
	if test "x$exec_prefix" = xNONE; then
		exec_prefix=$acl_final_prefix
	fi
	cmd="quicktime/ffmpeg/configure \$ff_conf_args \
		--prefix=\"\$acl_final_prefix\" \
		--libdir=\"$libdir\" --shlibdir=\"$libdir\""
	eval echo $cmd
	eval \$srcdir_c/$cmd
	)
fi
echo --------------------------------------------------
############# END run ffmpeg configure to generates avconfig.h

############## XIPH
PKG_CHECK_MODULES(libogg,ogg >= 1.1,[libogg=yes],:)
PKG_CHECK_MODULES(libvorbis,vorbis,[libvorbis=yes],:)
PKG_CHECK_MODULES(libvorbisenc,vorbisenc,[libvorbisenc=yes],:)
PKG_CHECK_MODULES(libvorbisfile,vorbisfile,[libvorbisfile=yes],:)
PKG_CHECK_MODULES(libtheora,theora,[libtheora=yes],:)
XIPH_CFLAGS="$libogg_CFLAGS $libvorbis_CFLAGS $libvorbisenc_CFLAGS $libvorbisfile_CFLAGS $libtheora_CFLAGS"
XIPH_LIBS="$libogg_LIBS $libvorbis_LIBS $libvorbisenc_LIBS $libvorbisfile_LIBS $libtheora_LIBS"
AC_SUBST(XIPH_CFLAGS)
AC_SUBST(XIPH_LIBS)
############## END OF XIPH

############## LAME
# Just test to see if we have lame installed.
AC_CHECK_HEADER(lame/lame.h,[libmp3lameh=yes])
AC_CHECK_LIB(mp3lame,lame_init,[libmp3lame=yes],,[$XIPH_LIBS])
# END LAME

############## OpenGL
AH_TEMPLATE(HAVE_GL, [Define to 1 if OpenGL 2.0 is available.])
AC_ARG_ENABLE(opengl,
	AC_HELP_STRING([--disable-opengl],
		[disables hardware accelerated rendering (default=autodetect, requires OpenGL 2.0)]),
	[ enable_opengl=$enableval ],
	[ enable_opengl=auto ])
if test "x$enable_opengl" = "xauto"; then
	AC_CHECK_LIB([GL], [glUseProgram],
		[OPENGL_LIBS="-lGL"; libGL=yes],
# On SUSE/OpenSUSE, NVidia places the OpenGL 2.0 capable library in /usr/X11R6/lib
# but it doesn't place a libGL.so there, so the linker won't pick it up
# we have to use the explicit libGL.so.1 path.
		save_LIBS="$LIBS"
		for l in /usr/X11R6/lib /usr/X11R6/lib64; do
			LIBS="$l/libGL.so.1"
			AC_MSG_CHECKING(for glUseProgram in $l/libGL.so.1)
			AC_TRY_LINK([],[extern int glUseProgram(); glUseProgram();],
				[OPENGL_LIBS="$l/libGL.so.1"; libGL=yes],[libGL=no])
			AC_MSG_RESULT([$libGL])
			test $libGL = yes && break
		done
		LIBS="$save_LIBS"
	)
	if test "x$libGL" = "xyes"; then
		enable_opengl=ok
		OPENGL_LIBS="-lGLU $OPENGL_LIBS"
		AC_DEFINE(HAVE_GL)
	fi
else
	if test "x$enable_opengl" = "xyes"; then
		enable_opengl=ok
		libGL=skip
		OPENGL_LIBS="-lGLU -lGL"
		AC_DEFINE(HAVE_GL)
	fi
fi
AC_SUBST(OPENGL_LIBS)
# END OpenGL

############## ask for extra debug output from X
AC_ARG_ENABLE(x-error-output,
	AC_HELP_STRING([--enable-x-error-output], [enable output when X report errors (default=no)])
	[],
	[if test yes = "$enableval" ; then
	AC_DEFINE(OUTPUT_X_ERROR, [1], [enable output when X report errors])
fi])
# END --enable-x-error-output

############## .png TO .o CONVERSION

AC_CHECK_TOOL(OBJCOPY, objcopy)
if test "x$OBJCOPY" = "x"; then
	AC_MSG_ERROR("objcopy from GNU binutils >= 2.11.90 not found")
fi
AC_CHECK_TOOL(OBJDUMP, objdump)
dnl extract target and architecture if objdump was found
if test "x$OBJDUMP" = "x"; then :; else
  AC_MSG_CHECKING(for object target)
  octarget=`$OBJDUMP --info | sed -ne '2p'` # extract first target
  AC_MSG_RESULT($octarget)
  AC_MSG_CHECKING(for object architecture)
  ocarch=`$OBJDUMP --info | sed -ne '4p'` # extract corresponding arch
  AC_MSG_RESULT($ocarch)
fi
AC_SUBST(OBJCOPYTARGET, $octarget)
AC_SUBST(OBJCOPYARCH, $ocarch)

############## END of .png TO .o CONVERSION


AC_OUTPUT(Makefile cinelerra-cvs-current.spec po/Makefile.in \
	m4/Makefile \
	libmpeg3/Makefile \
	libmpeg3/video/Makefile \
	libmpeg3/audio/Makefile \
	quicktime/Makefile \
	quicktime/encore50/Makefile \
	mpeg2enc/Makefile \
	toolame-02l/Makefile \
	guicast/Makefile \
	cinelerra/Makefile \
	cinelerra/data/Makefile \
	mplexlo/Makefile \
	plugins/colors/Makefile \
	plugins/libfourier/Makefile \
	plugins/libeffecttv/Makefile \
	plugins/Makefile \
	plugins/1080to540/Makefile \
	plugins/histogram/Makefile \
	plugins/720to480/Makefile \
	plugins/C41/Makefile \
	plugins/bandslide/Makefile \
	plugins/bandwipe/Makefile 
	plugins/blur/Makefile \
	plugins/bluebanana/Makefile \
	plugins/brightness/Makefile \
	plugins/burn/Makefile \
	plugins/parametric/Makefile \
	plugins/aging/Makefile \
	plugins/cdripper/Makefile \
	plugins/chromakey/Makefile \
	plugins/chromakeyhsv/Makefile \
	plugins/colorbalance/Makefile \
	plugins/color3way/Makefile \
	plugins/compressor/Makefile \
	plugins/crossfade/Makefile \
	plugins/decimate/Makefile \
	plugins/deinterlace/Makefile \
	plugins/delayaudio/Makefile \
	plugins/delayvideo/Makefile \
	plugins/denoise/Makefile \
	plugins/denoisefft/Makefile \
	plugins/denoisevideo/Makefile \
	plugins/denoisemjpeg/Makefile \
	plugins/despike/Makefile \
	plugins/dissolve/Makefile \
	plugins/dot/Makefile \
	plugins/fieldframe/Makefile \
	plugins/flip/Makefile \
	plugins/framefield/Makefile \
	plugins/findobject/Makefile \
	plugins/freeverb/Makefile \
	plugins/freezeframe/Makefile \
	plugins/gain/Makefile \
	plugins/gamma/Makefile \
	plugins/holo/Makefile \
	plugins/huesaturation/Makefile \
	plugins/interpolate/Makefile \
	plugins/interpolateaudio/Makefile \
	plugins/interpolatevideo/Makefile \
	plugins/invertaudio/Makefile \
	plugins/invertvideo/Makefile \
	plugins/irissquare/Makefile \
	plugins/ivtc/Makefile \
	plugins/lens/Makefile \
	plugins/liveaudio/Makefile \
	plugins/livevideo/Makefile \
	plugins/loopaudio/Makefile \
	plugins/loopvideo/Makefile \
	plugins/motion/Makefile \
	plugins/motionblur/Makefile \
	plugins/normalize/Makefile \
	plugins/oilpainting/Makefile \
	plugins/overlay/Makefile \
	plugins/overlayaudio/Makefile \
	plugins/pitch/Makefile \
	plugins/polar/Makefile \
	plugins/reroute/Makefile \
	plugins/reframert/Makefile \
	plugins/reframe/Makefile \
	plugins/resample/Makefile \
	plugins/reverb/Makefile \
	plugins/reverseaudio/Makefile \
	plugins/reversevideo/Makefile \
	plugins/rgb601/Makefile \
	plugins/rotate/Makefile \
	plugins/scale/Makefile \
	plugins/denoiseseltempavg/Makefile \
	plugins/shapewipe/Makefile \
	plugins/sharpen/Makefile \
	plugins/shiftinterlace/Makefile \
	plugins/slide/Makefile \
	plugins/spectrogram/Makefile \
	plugins/suv/Makefile \
	plugins/suv/data/Makefile \
	plugins/bluedottheme/Makefile \
	plugins/bluedottheme/data/Makefile \
	plugins/defaulttheme/Makefile \
	plugins/defaulttheme/data/Makefile \
	plugins/swapchannels/Makefile \
	plugins/synthesizer/Makefile \
	plugins/threshold/Makefile \
	plugins/timeavg/Makefile \
	plugins/timestretch/Makefile \
	plugins/titler/Makefile \
	plugins/translate/Makefile \
	plugins/unsharp/Makefile \
	plugins/videoscope/Makefile \
	plugins/wave/Makefile \
	plugins/whirl/Makefile \
	plugins/wipe/Makefile \
	plugins/yuv/Makefile \
	plugins/downsample/Makefile \
	plugins/flash/Makefile \
	plugins/gradient/Makefile \
	plugins/level/Makefile \
	plugins/linearblur/Makefile \
	plugins/perspective/Makefile \
	plugins/radialblur/Makefile \
	plugins/zoomblur/Makefile \
	plugins/timefront/Makefile \
	plugins/svg/Makefile \
	plugins/diffkey/Makefile \
	plugins/fonts/Makefile \
	image/Makefile)

succeeded=yes
AC_DEFUN([RPT],[if test "x$$1" = "xskip"; then status="not probed"
elif test "x$$1" = "xyes"; then status=found
else status=missing; succeeded=no
fi
echo "AC_HELP_STRING($2,$status)"])

echo
echo "Summary of mandatory components:"
RPT(libogg,libogg)
RPT(libvorbis,libvorbis)
RPT(libvorbisenc,libvorbisenc)
RPT(libvorbisfile,libvorbisfile)
RPT(libtheora,libtheora)
RPT(libOpenEXR,OpenEXR)
RPT(libdv,libdv)
RPT(libpng,libpng)
RPT(libjpeg,libjpeg libraries)
RPT(libjpegh,libjpeg headers)
RPT(libtiff,libtiff libraries)
RPT(libtiffh,libtiff headers)
RPT(freetype2,FreeType 2)
RPT(libx264,libx264 libraries)
RPT(libx264h,libx264 headers)
RPT(libuuid,libuuid libraries)
RPT(libuuidh,libuuid headers)
RPT(mjpegtools,mjpegtools)
RPT(libfftw3,libfftw3 libraries)
RPT(libfftw3h,libfftw3 headers)
RPT(liba52,liba52 libraries)
RPT(liba52h,liba52 headers)
RPT(libmp3lame,libmp3lame libraries)
RPT(libmp3lameh,libmp3lame headers)
RPT(libsndfile,libsndfile libraries)
RPT(libsndfileh,libsndfile headers)
RPT(libfaac,libfaac libraries)
RPT(libfaach,libfaac headers)
RPT(libfaad,libfaad libraries)
RPT(libfaadh,libfaad headers)
mandatory="$succeeded"

echo
echo "Summary of optional components:"
AC_DEFUN([ORPT],[if test "x$$1" = "xno"; then
	echo "$2 was opted out with $3"
else
	if test "x$$1" = "xok"; then component=enabled; else component=disabled; fi
	$4
	echo "$2 is $component"
fi])
ORPT(enable_esd,ESD (Enlightenment Sound Daemon),--disable-esd,RPT(esound,ESD subsystem))
ORPT(enable_alsa,ALSA,--disable-alsa,RPT(alsa,ALSA subsystem))
ORPT(enable_firewire,Firewire,--disable-firewire,
	RPT(libraw1394,libraw1394)
	RPT(libiec61883,libiec61883)
	RPT(libavc1394,libavc1394 libraries)
	RPT(libavc1394h,libavc1394 headers)
	RPT(librom1394,librom1394 libraries)
	RPT(librom1394h,librom1394 headers)
)
ORPT(enable_opengl,Hardware acceleration using OpenGL 2.0,--disable-opengl,
	RPT(libGL,OpenGL 2.0 libraries)
)
ORPT(enable_findobject,findobject plugin using OpenCV 2.0,--disable-findobject,
	RPT(opencv,opencv libraries)
	RPT(opencvh,opencv headers)
)

if test "x$mandatory" = "xno"; then
	echo
	echo "WARNING: Mandatory components are missing; compilation may fail!"
else
	echo
	echo "Now type"; echo "          make"; echo; echo "to start compilation."
fi
