bin_PROGRAMS = imgpacker
imgpacker_SOURCES =				\
	src/imgpack/application.cc		\
	src/imgpack/application.hh		\
	src/imgpack/main-window.cc		\
	src/imgpack/main-window.hh		\
	src/main.cc
imgpacker_CXXFLAGS = $(GTKMM_CFLAGS) -I$(top_srcdir)/src
imgpacker_LDADD = $(GTKMM_LIBS)

run: imgpacker
	$(builddir)/imgpacker
.PHONY: run
