####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = kmouth

## INCLUDES were found outside kdevelop specific part

kmouth_SOURCES = preferencesui.ui texttospeechconfigurationui.ui texttospeechconfigurationwidget.cpp optionsdialog.cpp configwizard.cpp phraseedit.cpp phraselist.cpp speech.cpp texttospeechsystem.cpp phraselistitem.cpp kmouth.cpp main.cpp 
kmouth_LDADD   = ./phrasebook/libphrasebook.a -lkdeprint $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET)

SUBDIRS = icons phrasebook books  . 

EXTRA_DIST = main.cpp kmouth.cpp kmouth.h kmouthui.rc kmouth.desktop phraselistitem.cpp phraselistitem.h texttospeechsystem.cpp texttospeechsystem.h speech.cpp speech.h phraselist.cpp phraselist.h phraseedit.cpp phraseedit.h kmouthrc configwizard.cpp configwizard.h optionsdialog.cpp optionsdialog.h texttospeechconfigurationwidget.h texttospeechconfigurationwidget.cpp texttospeechconfigurationui.ui preferencesui.ui 

install-data-local:
	$(mkinstalldirs) $(kde_appsdir)/Applications/
	$(INSTALL_DATA) $(srcdir)/kmouth.desktop $(kde_appsdir)/Applications/kmouth.desktop
	$(mkinstalldirs) $(kde_confdir)/
	$(INSTALL_DATA) $(srcdir)/kmouthrc $(kde_confdir)/kmouthrc

uninstall-local:
	-rm -f $(kde_appsdir)/Applications/kmouth.desktop
	-rm -f $(kde_confdir)/kmouthrc

####### kdevelop will overwrite this part!!! (end)############
# These paths are KDE specific. Use them:
# kde_appsdir         Where your application's menu entry (.desktop) should go to.
# kde_icondir         Where your icon should go to - better use KDE_ICON.
# kde_sounddir        Where your sounds should go to.
# kde_htmldir         Where your docs should go to. (contains lang subdirs)
# kde_datadir         Where you install application data. (Use a subdir)
# kde_locale          Where translation files should go to. (contains lang subdirs)
# kde_cgidir          Where cgi-bin executables should go to.
# kde_confdir         Where config files should go to (system-wide ones with default values).
# kde_mimedir         Where mimetypes .desktop files should go to.
# kde_servicesdir     Where services .desktop files should go to.
# kde_servicetypesdir Where servicetypes .desktop files should go to.
# kde_toolbardir      Where general toolbar icons should go to (deprecated, use KDE_ICON).
# kde_wallpaperdir    Where general wallpapers should go to.
# kde_templatesdir    Where templates for the "New" menu (Konqueror/KDesktop) should go to.
# kde_bindir          Where executables should go to. Use bin_PROGRAMS or bin_SCRIPTS.
# kde_libdir          Where shared libraries should go to. Use lib_LTLIBRARIES.
# kde_moduledir       Where modules (e.g. parts) should go to. Use kde_module_LTLIBRARIES.
# kde_styledir        Where Qt/KDE widget styles should go to (new in KDE 3).
# kde_designerdir     Where Qt Designer plugins should go to (new in KDE 3).

# set the include path for X, qt and KDE
INCLUDES= -Iphrasebook $(all_includes)

METASOURCES = AUTO

# the library search path. 
kmouth_LDFLAGS = $(all_libraries) $(KDE_RPATH)

rcdir = $(kde_datadir)/kmouth
rc_DATA = kmouthui.rc

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kmouth.pot; \
	fi

