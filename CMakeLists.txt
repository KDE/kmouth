cmake_minimum_required(VERSION 3.16)

# KDE Gear version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "22")
set (RELEASE_SERVICE_VERSION_MINOR "11")
set (RELEASE_SERVICE_VERSION_MICRO "90")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(kmouth VERSION ${RELEASE_SERVICE_VERSION})

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Wall -Wno-switch -O2 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wno-switch -O2")
    set(CMAKE_CXX_FLAGS_DEBUG  "-Wall -Wno-switch -g3 -ggdb -O0" CACHE STRING "Debug options." FORCE)
endif(NOT MSVC)

include(FeatureSummary)

set(QT_MIN_VERSION "5.15.2")
set(KF5_MIN_VERSION "5.90.0")


find_package(ECM ${KF5_MIN_VERSION} NO_MODULE REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMSetupVersion)
include(ECMAddAppIcon)
include(ECMInstallIcons)
include(ECMAddAppIcon)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    PrintSupport
    TextToSpeech
    Xml
    Widgets
)

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

ecm_setup_version(${kmouth_VERSION}
    VARIABLE_PREFIX KMOUTH
    VERSION_HEADER version.h
)

include_directories (${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )
add_definitions(
	#-DQT_DISABLE_DEPRECATED_BEFORE=0x050F00
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
    -DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055600
    -DKF_DEPRECATED_WARNINGS_SINCE=0x060000
)

add_subdirectory( doc )
add_subdirectory( icons )
add_subdirectory( books )

include_directories(./wordcompletion/  )


########### next target ###############
add_executable(kmouth)

target_sources(kmouth PRIVATE
    ./phrasebook/phrasebookreader.cpp
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

ki18n_wrap_ui(kmouth
    ./phrasebook/phrasebookdialog.ui
    ./wordcompletion/creationsourceui.ui
    ./wordcompletion/creationsourcedetailsui.ui
    ./wordcompletion/kdedocsourceui.ui
    ./wordcompletion/wordcompletionui.ui
    preferencesui.ui
    texttospeechconfigurationui.ui )


target_link_libraries(kmouth
                      KF5::Completion
                      KF5::ConfigCore
                      KF5::Crash
                      KF5::KIOCore
                      KF5::KIOWidgets
                      KF5::I18n
                      KF5::WidgetsAddons
                      KF5::XmlGui
                      Qt::PrintSupport
                      Qt::TextToSpeech
                      Qt::Widgets
                      Qt::Xml )

install(TARGETS kmouth  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS org.kde.kmouth.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )
install( FILES kmouthrc  DESTINATION  ${KDE_INSTALL_CONFDIR} )
install( FILES kmouthui.rc  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/kmouth )
install( FILES phrasebookdialogui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kmouth )
install(FILES org.kde.kmouth.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

ki18n_install(po)
kdoctools_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

