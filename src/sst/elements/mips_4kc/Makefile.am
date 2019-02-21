# -*- Makefile -*-
#
#

AM_CPPFLAGS = \
	$(MPI_CPPFLAGS) \
	-I$(top_srcdir)/src

compdir = $(pkglibdir)
comp_LTLIBRARIES = libmips_4kc.la
libmips_4kc_la_SOURCES = \
	mips_4kc.cc \
	mips_4kc.h \
	cl-cycle.cc \
	cl-cycle.h \
	spim-utils.cc \
	data.cc \
	reg.h


EXTRA_DIST = \
	README 


libmips_4kc_la_LDFLAGS = -module -avoid-version

##########################################################################
##########################################################################
##########################################################################


