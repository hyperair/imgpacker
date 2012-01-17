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
	src/imgpack/pixbuf-loader.hh		\
	src/imgpack/pixbuf-loader.cc		\
	src/imgpack/collage-viewer.hh		\
	src/imgpack/collage-viewer.cc		\
	src/main.cc
imgpacker_CXXFLAGS =						\
	$(CXX0X_FLAG)						\
	$(WARN_CXXFLAGS)					\
	$(GTKMM_CFLAGS)						\
	$(NIHPP_CFLAGS)						\
	-I$(top_srcdir)/src					\
	-DPROGRAMNAME_LOCALEDIR="\"$(PROGRAMNAME_LOCALEDIR)\""
imgpacker_LDADD = $(GTKMM_LIBS) -lasprintf

SUBDIRS = po

if ENABLE_WARNINGS
WARN_CXXFLAGS = -Wall -Wextra -Werror -pedantic
endif

run: imgpacker
	$(builddir)/imgpacker

update-po:
	$(MAKE) -C po update-po

DOC_INDICES = doc/html/index.html doc/xml/index.xml
$(DOC_INDICES): doc/Doxyfile $(imgpacker_SOURCES)
	doxygen $<
	touch $@

htmldocdir = $(docdir)/html
xmldocdir = $(docdir/xml

install-data-hook: $(DOC_INDICES)
	cd doc && find html xml -type d -print |		\
	while read dir; do					\
		$(mkinstalldirs) $(DESTDIR)$(docdir)/$$dir;	\
		$(install_sh_DATA)				\
			`find $$dir -maxdepth 1 -type f`	\
			$(DESTDIR)$(docdir)/$$dir;		\
	done


doc_DATA =					\
	doc/html/index.html			\
	doc/xml/index.xml

CLEANFILES = $(dir $(DOC_INDICES))

.PHONY: run update-po

ACLOCAL_AMFLAGS = -I m4
