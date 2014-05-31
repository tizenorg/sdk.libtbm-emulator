AM_CFLAGS = \
	@LIBTBM_EMULATOR_CFLAGS@ \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src

libtbm_emulator_la_LTLIBRARIES = libtbm_emulator.la
libtbm_emulator_ladir = /${bufmgr_dir}
libtbm_emulator_la_LIBADD = @LIBTBM_EMULATOR_LIBS@

libtbm_emulator_la_SOURCES = \
	tbm_bufmgr_emulator.c \
	tbm_emulator_log.c
