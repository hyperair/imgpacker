bin_PROGRAMS = imgpacker
imgpacker_SOURCES =				\
	src/imgpack/application.cc		\
	src/imgpack/application.hh		\
	src/imgpack/main-window.cc		\
	src/imgpack/main-window.hh		\
	src/imgpack/image-list.cc		\
	src/imgpack/image-list.hh		\
	src/imgpack/logger.hh			\
	src/imgpack/logger.cc			\
	src/imgpack/thread-pool.hh		\
	src/imgpack/thread-pool.cc		\
	src/main.cc
imgpacker_CXXFLAGS =						\
	-std=c++0x						\
	$(WARN_CXXFLAGS)					\
	$(GTKMM_CFLAGS)						\
	$(NIHPP_CFLAGS)						\
	-I$(top_srcdir)/src					\
	-DPROGRAMNAME_LOCALEDIR="\"$(PROGRAMNAME_LOCALEDIR)\""
imgpacker_LDADD = $(GTKMM_LIBS)

SUBDIRS = po

if ENABLE_WARNINGS
WARN_CXXFLAGS = -Wall -Wextra -Werror -pedantic
endif

run: imgpacker
	$(builddir)/imgpacker

update-po:
	$(MAKE) -C po update-po

.PHONY: run update-po
