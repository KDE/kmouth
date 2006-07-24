
add_subdirectory( history ) 
add_subdirectory( phrase ) 
add_subdirectory( phrasebook ) 
add_subdirectory( app ) 




########### install files ###############

kde4_install_icons( ${DATA_INSTALL_DIR}/kmouth/icons hicolor )



#original Makefile.am contents follow:

######## kdevelop will overwrite this part!!! (begin)##########
#
#SUBDIRS = history phrase phrasebook app 
#
#EXTRA_DIST = hi16-action-speak.png hi32-action-speak.png hi48-action-speak.png hi22-action-speak.png 
#
######## kdevelop will overwrite this part!!! (end)############
#
#kmouthiconsdir = $(kde_datadir)/kmouth/icons
#kmouthicons_ICON = AUTO
#
