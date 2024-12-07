cmake_minimum_required(VERSION 3.16)

# KDE Gear version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "25")
set (RELEASE_SERVICE_VERSION_MINOR "03")
set (RELEASE_SERVICE_VERSION_MICRO "70")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(kmouth VERSION ${RELEASE_SERVICE_VERSION})
set(QT_REQUIRED_VERSION "6.6.0")
set(KF_MIN_VERSION "6.3.0")


if(NOT MSVC)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Wall -Wno-switch -O2 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wno-switch -O2")
    set(CMAKE_CXX_FLAGS_DEBUG  "-Wall -Wno-switch -g3 -ggdb -O0" CACHE STRING "Debug options." FORCE)
endif(NOT MSVC)

include(FeatureSummary)

find_package(ECM ${KF_MIN_VERSION} NO_MODULE REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} )
include(ECMAddAppIcon)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMDeprecationSettings)

include(KDEClangFormat)
include(KDEGitCommitHooks)

find_package(Qt6 ${QT_REQUIRED_VERSION} REQUIRED COMPONENTS
    Core
    PrintSupport
    TextToSpeech
    Xml
    Widgets
)

find_package(KF6  ${KF_MIN_VERSION} REQUIRED COMPONENTS
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
ecm_set_disabled_deprecation_versions(QT 6.8.0  KF 6.5.0)

add_subdirectory( doc )
add_subdirectory( icons )
add_subdirectory( books )

include_directories(./wordcompletion/  )


########### next target ###############
add_executable(kmouth)

target_sources(kmouth PRIVATE
    ./phrasebook/phrasebookreader.cpp ./phrasebook/phrasebookreader.h
    ./phrasebook/phrasebookdialog.cpp ./phrasebook/phrasebookdialog.h
    ./phrasebook/initialphrasebookwidget.cpp ./phrasebook/initialphrasebookwidget.h
    ./phrasebook/phrasebook.cpp ./phrasebook/phrasebook.h
    ./wordcompletion/wordlist.cpp ./wordcompletion/wordlist.h
    ./wordcompletion/wordcompletion.cpp ./wordcompletion/wordcompletion.h
    ./wordcompletion/wordcompletionwidget.cpp ./wordcompletion/wordcompletionwidget.h
    ./wordcompletion/dictionarycreationwizard.cpp ./wordcompletion/dictionarycreationwizard.h
    texttospeechconfigurationwidget.cpp texttospeechconfigurationwidget.h
    optionsdialog.cpp optionsdialog.h
    configwizard.cpp configwizard.h
    phraselist.cpp phraselist.h
    speech.cpp speech.h
    texttospeechsystem.cpp texttospeechsystem.h
    kmouth.cpp kmouth.h
    main.cpp

    kmouth.qrc
)

ki18n_wrap_ui(kmouth
    ./phrasebook/phrasebookdialog.ui
    ./wordcompletion/creationsourceui.ui
    ./wordcompletion/creationsourcedetailsui.ui
    ./wordcompletion/kdedocsourceui.ui
    ./wordcompletion/wordcompletionui.ui
    preferencesui.ui
    texttospeechconfigurationui.ui )


target_link_libraries(kmouth
                      KF6::Completion
                      KF6::ConfigCore
                      KF6::Crash
                      KF6::KIOCore
                      KF6::KIOWidgets
                      KF6::I18n
                      KF6::WidgetsAddons
                      KF6::XmlGui
                      Qt::PrintSupport
                      Qt::TextToSpeech
                      Qt::Widgets
                      Qt::Xml )

install(TARGETS kmouth  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS org.kde.kmouth.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )
install( FILES kmouthrc  DESTINATION  ${KDE_INSTALL_CONFDIR} )
install(FILES org.kde.kmouth.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

ki18n_install(po)
kdoctools_install(po)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

