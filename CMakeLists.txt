cmake_minimum_required(VERSION 2.8.12)

project(kmouth)

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Wall -Wno-switch -O2 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wno-switch -O2")
    set(CMAKE_CXX_FLAGS_DEBUG  "-Wall -Wno-switch -g3 -ggdb -O0" CACHE STRING "Debug options." FORCE)
endif(NOT MSVC)

include(FeatureSummary)

set(QT_MIN_VERSION "5.9.0")
set(KF5_MIN_VERSION "5.46.0")


find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core PrintSupport TextToSpeech Xml Widgets)

find_package(ECM ${KF5_MIN_VERSION} NO_MODULE REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(ECMAddAppIcon)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(ECMAddAppIcon)

find_package(KF5  ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Crash
    Completion
    Config
    ConfigWidgets
    CoreAddons
    DocTools
    I18n
    KIO
    WidgetsAddons
    XmlGui
)
include_directories (${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
   add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)
   add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x060000)
endif()
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
                      KF5::Completion
                      KF5::ConfigCore
                      KF5::Crash
                      KF5::KIOCore
                      KF5::KIOWidgets
                      KF5::I18n
                      KF5::WidgetsAddons
                      KF5::XmlGui
                      Qt5::PrintSupport
                      Qt5::TextToSpeech
                      Qt5::Widgets
                      Qt5::Xml )

install(TARGETS kmouth  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS org.kde.kmouth.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kmouthrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
install( FILES kmouthui.rc  DESTINATION  ${KXMLGUI_INSTALL_DIR}/kmouth )
install( FILES phrasebookdialogui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kmouth )
install(FILES org.kde.kmouth.appdata.xml DESTINATION ${CMAKE_INSTALL_METAINFODIR})

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

