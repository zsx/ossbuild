JRTP_DIR=jrtp

if USE_GSM
GSM_DIR=gsm
else
GSM_DIR=
endif

#if USE_LIBJ2K
#J2K_DIR=jpeg2000
#else
J2K_DIR=
#endif

#if USE_LIBJASPER
#JASPER_DIR=jasper
#else
JASPER_DIR=
#endif

if USE_JINGLEP2P
ICE_DIR=ice
else
ICE_DIR=
endif

if USE_GCONF
GCONF_DIR=gconf
else
GCONF_DIR=
endif

SUBDIRS = \
$(JRTP_DIR) \
$(GCONF_DIR) \
$(MIMIC_DIR) \
$(J2K_DIR) \
$(JASPER_DIR) \
$(ICE_DIR) \
$(GSM_DIR)

DIST_SUBDIRS = gsm ice jasper jpeg2000 jrtp gconf
