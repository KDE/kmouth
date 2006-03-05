kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kmouth FILES phrasebookdialogui.rc )

kde4_footer()



#original Makefile.am contents follow:

######## kdevelop will overwrite this part!!! (begin)##########
#noinst_LIBRARIES = libphrasebook.a
#
#INCLUDES = $(all_includes)
#
#libphrasebook_a_METASOURCES = AUTO
#
#libphrasebook_a_SOURCES = buttonboxui.ui phrasetree.cpp phrasebookparser.cpp phrasebookdialog.cpp phrasebook.cpp 
#
#
#EXTRA_DIST = phrasebook.cpp phrasebook.h phrasebookdialog.cpp phrasebookdialog.h phrasebookparser.cpp phrasebookparser.h phrasetree.cpp phrasetree.h phrasebookdialogui.rc buttonboxui.ui 
#
######## kdevelop will overwrite this part!!! (end)############
#
#rcdir = $(kde_datadir)/kmouth
#rc_DATA = phrasebookdialogui.rc
