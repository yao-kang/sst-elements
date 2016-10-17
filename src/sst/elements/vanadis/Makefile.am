
AM_CPPFLAGS = \
	$(BOOST_CPPFLAGS) \
	$(MPI_CPPFLAGS) 

compdir = $(pkglibdir)
comp_LTLIBRARIES = libvanadis.la

libvanadis_la_SOURCES = \
	vcore.cc \
	vcore.h \
	icreader.cc \
	icreader.h	

libvanadis_la_LDFLAGS = -module -avoid-version
