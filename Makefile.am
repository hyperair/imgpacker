bin_PROGRAMS = imgpacker
EXTRA_PROGRAMS = imgpacker-unity

imgpacker_SOURCES =				\
	src/imgpack/application.cc		\
	src/imgpack/application.hh		\
	src/imgpack/util/logger.hh		\
	src/imgpack/util/logger.cc		\
	src/imgpack/util/thread-pool.hh		\
	src/imgpack/util/thread-pool.cc		\
	src/imgpack/util/async-operation.hh	\
	src/imgpack/util/async-operation.cc	\
	src/imgpack/gtkui/gtk-application.cc	\
	src/imgpack/gtkui/gtk-application.hh	\
	src/imgpack/gtkui/main-window.cc	\
	src/imgpack/gtkui/main-window.hh	\
	src/imgpack/gtkui/image-list.cc		\
	src/imgpack/gtkui/image-list.hh		\
	src/imgpack/gtkui/pixbuf-loader.hh	\
	src/imgpack/gtkui/pixbuf-loader.cc	\
	src/imgpack/gtkui/collage-viewer.hh	\
	src/imgpack/gtkui/collage-viewer.cc	\
	src/imgpack/gtkui/collage-treeview.hh	\
	src/imgpack/gtkui/collage-treeview.cc	\
	src/imgpack/algorithm/bin-packer.hh	\
	src/imgpack/algorithm/bin-packer.cc	\
	src/main.cc

imgpacker_unity_SOURCES = unity-build.cc
imgpacker_unity_CXXFLAGS = $(imgpacker_CXXFLAGS)
imgpacker_unity_LDADD = $(imgpacker_LDADD)

unity-build.cc: Makefile
	for file in $(filter %.cc, $(imgpacker_SOURCES)); do	\
		echo "#include <$$file>";			\
	done > $@

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

CLEANFILES = $(dir $(DOC_INDICES)) unity-build.cc

.PHONY: run update-po

ACLOCAL_AMFLAGS = -I m4
