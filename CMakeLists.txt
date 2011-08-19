project(kmouth)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)
set(libphrasebook_SRCS 
	./phrasebook/phrasetree.cpp
	./phrasebook/phrasebookparser.cpp
	./phrasebook/phrasebookdialog.cpp
	./phrasebook/phrasebook.cpp
	)

kde4_add_ui_files(libphrasebook_SRCS ./phrasebook/buttonboxui.ui) 

set(libwordcompletion_SRCS
	./wordcompletion/wordlist.cpp 
	./wordcompletion/wordcompletion.cpp 
	./wordcompletion/wordcompletionwidget.cpp 
	./wordcompletion/dictionarycreationwizard.cpp 
	)
        
kde4_add_ui_files(libwordcompletion_SRCS 
	./wordcompletion/creationsourceui.ui 
	./wordcompletion/creationsourcedetailsui.ui 
	./wordcompletion/kdedocsourceui.ui
	./wordcompletion/wordcompletionui.ui )

add_subdirectory( doc )
add_subdirectory( icons ) 
add_subdirectory( phrasebook ) 
add_subdirectory( books ) 

include_directories(./wordcompletion/  )


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

set(kspeech_xml  ${KDE4_DBUS_INTERFACES_DIR}/org.kde.KSpeech.xml)
qt4_add_dbus_interface(kmouth_SRCS ${kspeech_xml} kspeech_interface)

kde4_add_ui_files(kmouth_SRCS preferencesui.ui texttospeechconfigurationui.ui )

kde4_add_executable(kmouth ${kmouth_SRCS})

target_link_libraries(kmouth  ${KDE4_KIO_LIBS} ${KDE4_KDE3SUPPORT_LIBS} )

install(TARGETS kmouth  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS kmouth.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kmouthrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
install( FILES kmouthui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kmouth )



