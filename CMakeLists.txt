cmake_minimum_required(VERSION 2.8.12)

project(kmouth)

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Wall -Wno-switch -O2 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wno-switch -O2")
    set(CMAKE_CXX_FLAGS_DEBUG  "-Wall -Wno-switch -g3 -ggdb -O0" CACHE STRING "Debug options." FORCE)
endif(NOT MSVC)

include(FeatureSummary)
find_package(Qt5 REQUIRED COMPONENTS Core TextToSpeech Xml Widgets)

find_package(ECM 1.7.0 NO_MODULE REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(ECMAddAppIcon)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMInstallIcons)
include(ECMAddAppIcon)

find_package(KF5 REQUIRED COMPONENTS
    I18n
    Crash
    Config
    ConfigWidgets
    CoreAddons
    DocTools
    KIO
    KDELibs4Support
)
include_directories (${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

add_subdirectory( doc )
add_subdirectory( icons )
add_subdirectory( books )

include_directories(./wordcompletion/  )


########### next target ###############

set(kmouth_SRCS
    ./phrasebook/phrasebookparser.cpp
    ./phrasebook/phrasebookdialog.cpp
    ./phrasebook/initialphrasebookwidget.cpp
    ./phrasebook/phrasebook.cpp
    ./wordcompletion/wordlist.cpp
    ./wordcompletion/wordcompletion.cpp
    ./wordcompletion/wordcompletionwidget.cpp
    ./wordcompletion/dictionarycreationwizard.cpp
    texttospeechconfigurationwidget.cpp
    optionsdialog.cpp
    configwizard.cpp
    phraselist.cpp
    speech.cpp
    texttospeechsystem.cpp
    kmouth.cpp
    main.cpp )

ki18n_wrap_ui(kmouth_SRCS
    ./phrasebook/phrasebookdialog.ui
    ./wordcompletion/creationsourceui.ui
    ./wordcompletion/creationsourcedetailsui.ui
    ./wordcompletion/kdedocsourceui.ui
    ./wordcompletion/wordcompletionui.ui
    preferencesui.ui
    texttospeechconfigurationui.ui )

add_executable(kmouth ${kmouth_SRCS})

target_link_libraries(kmouth
                      KF5::ConfigCore
                      KF5::KIOCore
                      KF5::I18n
                      KF5::KDELibs4Support
                      Qt5::TextToSpeech
                      Qt5::Widgets )

install(TARGETS kmouth  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS kmouth.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kmouthrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
install( FILES kmouthui.rc  DESTINATION  ${KXMLGUI_INSTALL_DIR}/kmouth )
install( FILES phrasebookdialogui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kmouth )



