bin_PROGRAMS = imgpacker
imgpacker_SOURCES =				\
	src/imgpack/imgpackapplication.cc	\
	src/imgpack/imgpackapplication.hh	\
	src/main.cc
imgpacker_CXXFLAGS = $(GTKMM_CFLAGS) -I$(top_srcdir)/src
imgpacker_LDADD = $(GTKMM_LIBS)

run: imgpacker
	$(builddir)/imgpacker
.PHONY: run
