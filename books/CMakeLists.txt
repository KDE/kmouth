



########### install files ###############

install( FILES de.desktop de-courteousness.phrasebook de-greetings.phrasebook de-howareyou.phrasebook de-personal.phrasebook  DESTINATION  ${DATA_INSTALL_DIR}/kmouth/books/de )
install( FILES sv.desktop sv-courteousness.phrasebook sv-greetings.phrasebook sv-howareyou.phrasebook sv-personal.phrasebook  DESTINATION  ${DATA_INSTALL_DIR}/kmouth/books/sv )
install( FILES en.desktop en-courteousness.phrasebook en-greetings.phrasebook en-howareyou.phrasebook en-personal.phrasebook  DESTINATION  ${DATA_INSTALL_DIR}/kmouth/books/en )




#original Makefile.am contents follow:

#
#de_DATA = de.desktop de-courteousness.phrasebook de-greetings.phrasebook de-howareyou.phrasebook de-personal.phrasebook
#dedir =  $(kde_datadir)/kmouth/books/de
#
#en_DATA = en.desktop en-courteousness.phrasebook en-greetings.phrasebook en-howareyou.phrasebook en-personal.phrasebook
#endir =  $(kde_datadir)/kmouth/books/en
#
#sv_DATA = sv.desktop sv-courteousness.phrasebook sv-greetings.phrasebook sv-howareyou.phrasebook sv-personal.phrasebook
#svdir =  $(kde_datadir)/kmouth/books/sv
