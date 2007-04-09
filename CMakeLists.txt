project(kmouth)

set(libphrasebook_SRCS 
	${CMAKE_SOURCE_DIR}/kmouth/phrasebook/phrasetree.cpp
	${CMAKE_SOURCE_DIR}/kmouth/phrasebook/phrasebookparser.cpp
	${CMAKE_SOURCE_DIR}/kmouth/phrasebook/phrasebookdialog.cpp
	${CMAKE_SOURCE_DIR}/kmouth/phrasebook/phrasebook.cpp
	)

kde4_add_ui_files(libphrasebook_SRCS ${CMAKE_SOURCE_DIR}/kmouth/phrasebook/buttonboxui.ui) 

set(libwordcompletion_SRCS
	${CMAKE_SOURCE_DIR}/kmouth/wordcompletion/wordlist.cpp 
	${CMAKE_SOURCE_DIR}/kmouth/wordcompletion/wordcompletion.cpp 
	${CMAKE_SOURCE_DIR}/kmouth/wordcompletion/wordcompletionwidget.cpp 
	${CMAKE_SOURCE_DIR}/kmouth/wordcompletion/dictionarycreationwizard.cpp 
	${CMAKE_SOURCE_DIR}/kmouth/wordcompletion/klanguagebutton.cpp
	${CMAKE_SOURCE_DIR}/kmouth/wordcompletion/klanguagebuttonhelper.cpp
	)
        
kde4_add_ui_files(libwordcompletion_SRCS 
	${CMAKE_SOURCE_DIR}/kmouth/wordcompletion/creationsourceui.ui 
	${CMAKE_SOURCE_DIR}/kmouth/wordcompletion/creationsourcedetailsui.ui 
	${CMAKE_SOURCE_DIR}/kmouth/wordcompletion/kdedocsourceui.ui
	${CMAKE_SOURCE_DIR}/kmouth/wordcompletion/wordcompletionui.ui )

add_subdirectory( icons ) 
add_subdirectory( phrasebook ) 
add_subdirectory( wordcompletion ) 
add_subdirectory( books ) 

include_directories(${CMAKE_SOURCE_DIR}/kmouth/wordcompletion/  )


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

set(kspeech_xml  ${DBUS_INTERFACES_DIR}/org.kde.KSpeech.xml)
qt4_add_dbus_interface(kmouth_SRCS ${kspeech_xml} kspeech_interface)
kde4_automoc(${kmouth_SRCS})

kde4_add_ui_files(kmouth_SRCS preferencesui.ui texttospeechconfigurationui.ui )

kde4_add_executable(kmouth ${kmouth_SRCS})

target_link_libraries(kmouth  ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KDEPRINT_LIBS} ${KDE4_KSPELL2_LIBS}  )

install(TARGETS kmouth  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kmouth.desktop  DESTINATION  ${XDG_APPS_DIR} )
install( FILES kmouthrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
install( FILES kmouthui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kmouth )



