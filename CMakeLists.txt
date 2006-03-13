kde4_header()

set(libphrasebook_SRCS 
	${CMAKE_SOURCE_DIR}/kmouth/phrasebook/phrasetree.cpp
	${CMAKE_SOURCE_DIR}/kmouth/phrasebook/phrasebookparser.cpp
	${CMAKE_SOURCE_DIR}/kmouth/phrasebook/phrasebookdialog.cpp
	${CMAKE_SOURCE_DIR}/kmouth/phrasebook/phrasebook.cpp
	)

kde4_add_ui3_files(libphrasebook_SRCS ${CMAKE_SOURCE_DIR}/kmouth/phrasebook/buttonboxui.ui)


set(libwordcompletion_SRCS
	${CMAKE_SOURCE_DIR}/kmouth/wordcompletion/wordlist.cpp 
	${CMAKE_SOURCE_DIR}/kmouth/wordcompletion/wordcompletion.cpp 
	${CMAKE_SOURCE_DIR}/kmouth/wordcompletion/wordcompletionwidget.cpp 
	${CMAKE_SOURCE_DIR}/kmouth/wordcompletion/dictionarycreationwizard.cpp 
	${CMAKE_SOURCE_DIR}/kmouth/wordcompletion/klanguagebuttonhelper.cpp
	)
kde4_add_ui3_files(libwordcompletion_SRCS 
	${CMAKE_SOURCE_DIR}/kmouth/wordcompletion/wordcompletionui.ui 
	${CMAKE_SOURCE_DIR}/kmouth/wordcompletion/creationsourceui.ui 
	${CMAKE_SOURCE_DIR}/kmouth/wordcompletion/creationsourcedetailsui.ui 
	${CMAKE_SOURCE_DIR}/kmouth/wordcompletion/kdedocsourceui.ui
	)

add_subdirectory( icons ) 
add_subdirectory( phrasebook ) 
add_subdirectory( wordcompletion ) 
add_subdirectory( books ) 

include_directories(${CMAKE_SOURCE_DIR}/kmouth/wordcompletion/ ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kmouth_SRCS ${libwordcompletion_SRCS} ${libphrasebook_SRCS}
   texttospeechconfigurationwidget.cpp 
   optionsdialog.cpp 
   configwizard.cpp 
   phraseedit.cpp 
   phraselist.cpp 
   speech.cpp 
   texttospeechsystem.cpp 
   phraselistitem.cpp 
   kmouth.cpp 
   main.cpp )

kde4_automoc(${kmouth_SRCS})

kde4_add_ui3_files(kmouth_SRCS preferencesui.ui texttospeechconfigurationui.ui )

kde4_add_executable(kmouth ${kmouth_SRCS})

target_link_libraries(kmouth  ${QT_AND_KDECORE_LIBS} kdeprint kdeui kspell2 kde3support )

install_targets(/bin kmouth )


########### install files ###############

install_files( ${APPLNK_INSTALL_DIR}/Applications FILES kmouth.desktop )
install_files( ${CONFIG_INSTALL_DIR} FILES kmouthrc )
install_files( ${DATA_INSTALL_DIR}/kmouth FILES kmouthui.rc )

kde4_footer()



#original Makefile.am contents follow:

#bin_PROGRAMS = kmouth
#
#kmouth_SOURCES = preferencesui.ui texttospeechconfigurationui.ui texttospeechconfigurationwidget.cpp optionsdialog.cpp configwizard.cpp phraseedit.cpp phraselist.cpp speech.cpp texttospeechsystem.cpp phraselistitem.cpp kmouth.cpp main.cpp 
#kmouth_LDADD   = ./phrasebook/libphrasebook.a ./wordcompletion/libwordcompletion.a -lkdeprint $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET)
#
#SUBDIRS = icons phrasebook wordcompletion books . 
#
#EXTRA_DIST = main.cpp version.h kmouth.cpp kmouth.h kmouthui.rc kmouth.desktop phraselistitem.cpp phraselistitem.h texttospeechsystem.cpp texttospeechsystem.h speech.cpp speech.h phraselist.cpp phraselist.h phraseedit.cpp phraseedit.h kmouthrc configwizard.cpp configwizard.h optionsdialog.cpp optionsdialog.h texttospeechconfigurationwidget.h texttospeechconfigurationwidget.cpp texttospeechconfigurationui.ui preferencesui.ui 
#
#kde_conf_DATA = kmouthrc
#
#apps_DATA = kmouth.desktop
#appsdir = $(kde_appsdir)/Applications
#
## set the include path for X, qt and KDE
#INCLUDES= -Iphrasebook -Iwordcompletion $(all_includes)
#
#METASOURCES = AUTO
#
## the library search path. 
#kmouth_LDFLAGS = $(all_libraries) -lkspell2 -lkde3support $(KDE_RPATH)
#
#rcdir = $(kde_datadir)/kmouth
#rc_DATA = kmouthui.rc
#
#messages: rc.cpp
#	$(EXTRACTRC) *.rc *.ui */*.ui >> ./rc.cpp
#	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
#	if test -n "$$LIST"; then \
#	 $(XGETTEXT) $$LIST -o $(podir)/kmouth.pot; \
#	fi
#