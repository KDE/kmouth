bin_PROGRAMS = kmouth

kmouth_SOURCES = preferencesui.ui texttospeechconfigurationui.ui texttospeechconfigurationwidget.cpp optionsdialog.cpp configwizard.cpp phraseedit.cpp phraselist.cpp speech.cpp texttospeechsystem.cpp phraselistitem.cpp kmouth.cpp main.cpp 
kmouth_LDADD   = ./phrasebook/libphrasebook.a ./wordcompletion/libwordcompletion.a -lkdeprint $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET)

SUBDIRS = icons phrasebook wordcompletion books . 

EXTRA_DIST = main.cpp version.h kmouth.cpp kmouth.h kmouthui.rc kmouth.desktop phraselistitem.cpp phraselistitem.h texttospeechsystem.cpp texttospeechsystem.h speech.cpp speech.h phraselist.cpp phraselist.h phraseedit.cpp phraseedit.h kmouthrc configwizard.cpp configwizard.h optionsdialog.cpp optionsdialog.h texttospeechconfigurationwidget.h texttospeechconfigurationwidget.cpp texttospeechconfigurationui.ui preferencesui.ui 

install-data-local:
	$(mkinstalldirs) $(kde_appsdir)/Applications/
	$(INSTALL_DATA) $(srcdir)/kmouth.desktop $(kde_appsdir)/Applications/kmouth.desktop
	$(mkinstalldirs) $(kde_confdir)/
	$(INSTALL_DATA) $(srcdir)/kmouthrc $(kde_confdir)/kmouthrc

uninstall-local:
	-rm -f $(kde_appsdir)/Applications/kmouth.desktop
	-rm -f $(kde_confdir)/kmouthrc


# set the include path for X, qt and KDE
INCLUDES= -Iphrasebook -Iwordcompletion $(all_includes)

METASOURCES = AUTO

# the library search path. 
kmouth_LDFLAGS = $(all_libraries) -lkspell $(KDE_RPATH)

rcdir = $(kde_datadir)/kmouth
rc_DATA = kmouthui.rc

messages: rc.cpp
	$(EXTRACTRC) *.rc *.ui */*.ui >> ./rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kmouth.pot; \
	fi

