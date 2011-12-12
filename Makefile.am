bin_PROGRAMS = imgpacker
imgpacker_SOURCES =				\
	src/imgpack/application.cc		\
	src/imgpack/application.hh		\
	src/imgpack/main-window.cc		\
	src/imgpack/main-window.hh		\
	src/main.cc
imgpacker_CXXFLAGS =						\
	$(GTKMM_CFLAGS)						\
	-I$(top_srcdir)/src					\
	-DPROGRAMNAME_LOCALEDIR="\"$(PROGRAMNAME_LOCALEDIR)\""
imgpacker_LDADD = $(GTKMM_LIBS)

SUBDIRS = po

run: imgpacker
	$(builddir)/imgpacker

update-po:
	$(MAKE) -C po update-po

.PHONY: run update-po
