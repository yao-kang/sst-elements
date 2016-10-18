
AM_CPPFLAGS = \
	$(BOOST_CPPFLAGS) \
	$(MPI_CPPFLAGS) 

compdir = $(pkglibdir)
comp_LTLIBRARIES = libvanadis.la

libvanadis_la_SOURCES = \
	vcore.cc \
	vcore.h \
	icreader/icreader.cc \
	icreader/icreader.h \
	libvanadis.cc \
	elf/elfreader.h

libvanadis_la_LDFLAGS = -module -avoid-version
