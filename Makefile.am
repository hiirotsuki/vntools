bin_PROGRAMS = \
	opk_pack \
	opk_unpack \
	cromwell_pak_unpack \
	nekopunch_unpack \
	arcx_unpack \
	mif_unpack \
	ng3_unpack \
	cgd2bmp \
	ggp_decrypt \
	sm2mpx10_unpack \
	sm2mpx10_pack \
	ifl_unpack \
	ifl_pack \
	succubus_arc_unpack \
	studio-sakura_unpack

opk_pack_SOURCES = cromwell/opk_pack.c
opk_unpack_SOURCES = cromwell/opk_unpack.c
cromwell_pak_unpack_SOURCES = cromwell/pak_unpack.c
cromwell_pak_unpack_LDADD = -lz

nekopunch_unpack_SOURCES = nekopunch/nekopunch_unpack.c
nekopunch_unpack_LDADD = lib/libcommon.a

arcx_unpack_SOURCES = ARCX/arcx_unpack.c
arcx_unpack_LDADD = lib/libcommon.a

mif_unpack_SOURCES = basil/mif_unpack.c
ng3_unpack_SOURCES = basil/ng3_unpack.c

cgd2bmp_SOURCES = carriere/cgd2bmp.c

ggp_decrypt_SOURCES = ikura/ggp_decrypt.c
sm2mpx10_unpack_SOURCES = ikura/sm2mpx10_unpack.c
sm2mpx10_pack_SOURCES = ikura/sm2mpx10_pack.c

ifl_unpack_SOURCES = silky/ifl_unpack.c
ifl_pack_SOURCES = silky/ifl_pack.c

succubus_arc_unpack_SOURCES = succubus/succubus_arc_unpack.c

studio_sakura_unpack_SOURCES = studio-sakura/studio-sakura_unpack.c
studio_sakura_unpack_LDADD = lib/libcommon.a

AM_CPPFLAGS = -I$(top_srcdir)/lib

SUBDIRS = lib
