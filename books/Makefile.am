EXTRA_DIST = de.desktop                  en.desktop \
             de-courteousness.phrasebook en-courteousness.phrasebook \
             de-greetings.phrasebook     en-greetings.phrasebook \
             de-howareyou.phrasebook     en-howareyou.phrasebook \
             de-personal.phrasebook      en-personal.phrasebook

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kmouth/books/de/
	$(INSTALL_DATA) $(srcdir)/de.desktop $(kde_datadir)/kmouth/books/de/.directory
	$(INSTALL_DATA) $(srcdir)/de-courteousness.phrasebook $(kde_datadir)/kmouth/books/de/courteousness.phrasebook
	$(INSTALL_DATA) $(srcdir)/de-greetings.phrasebook $(kde_datadir)/kmouth/books/de/greetings.phrasebook
	$(INSTALL_DATA) $(srcdir)/de-howareyou.phrasebook $(kde_datadir)/kmouth/books/de/howareyou.phrasebook
	$(INSTALL_DATA) $(srcdir)/de-personal.phrasebook $(kde_datadir)/kmouth/books/de/personal.phrasebook
	$(mkinstalldirs) $(kde_datadir)/kmouth/books/en/
	$(INSTALL_DATA) $(srcdir)/en.desktop $(kde_datadir)/kmouth/books/en/.directory
	$(INSTALL_DATA) $(srcdir)/en-courteousness.phrasebook $(kde_datadir)/kmouth/books/en/courteousness.phrasebook
	$(INSTALL_DATA) $(srcdir)/en-greetings.phrasebook $(kde_datadir)/kmouth/books/en/greetings.phrasebook
	$(INSTALL_DATA) $(srcdir)/en-howareyou.phrasebook $(kde_datadir)/kmouth/books/en/howareyou.phrasebook
	$(INSTALL_DATA) $(srcdir)/en-personal.phrasebook $(kde_datadir)/kmouth/books/en/personal.phrasebook

uninstall-local:
	-rm -f $(kde_datadir)/kmouth/books/de/.directory
	-rm -f $(kde_datadir)/kmouth/books/de/courteousness.phrasebook
	-rm -f $(kde_datadir)/kmouth/books/de/greetings.phrasebook
	-rm -f $(kde_datadir)/kmouth/books/de/howareyou.phrasebook
	-rm -f $(kde_datadir)/kmouth/books/de/personal.phrasebook
	-rm -f $(kde_datadir)/kmouth/books/en/.directory
	-rm -f $(kde_datadir)/kmouth/books/en/courteousness.phrasebook
	-rm -f $(kde_datadir)/kmouth/books/en/greetings.phrasebook
	-rm -f $(kde_datadir)/kmouth/books/en/howareyou.phrasebook
	-rm -f $(kde_datadir)/kmouth/books/en/personal.phrasebook
