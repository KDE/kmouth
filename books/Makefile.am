EXTRA_DIST = de.desktop                  en.desktop                  sv.desktop \
             de-courteousness.phrasebook en-courteousness.phrasebook sv-courteousness.phrasebook \
             de-greetings.phrasebook     en-greetings.phrasebook     sv-greetings.phrasebook \
             de-howareyou.phrasebook     en-howareyou.phrasebook     sv-howareyou.phrasebook \
             de-personal.phrasebook      en-personal.phrasebook      sv-personal.phrasebook

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
	$(mkinstalldirs) $(kde_datadir)/kmouth/books/sv/
	$(INSTALL_DATA) $(srcdir)/sv.desktop $(kde_datadir)/kmouth/books/sv/.directory
	$(INSTALL_DATA) $(srcdir)/sv-courteousness.phrasebook $(kde_datadir)/kmouth/books/sv/courteousness.phrasebook
	$(INSTALL_DATA) $(srcdir)/sv-greetings.phrasebook $(kde_datadir)/kmouth/books/sv/greetings.phrasebook
	$(INSTALL_DATA) $(srcdir)/sv-howareyou.phrasebook $(kde_datadir)/kmouth/books/sv/howareyou.phrasebook
	$(INSTALL_DATA) $(srcdir)/sv-personal.phrasebook $(kde_datadir)/kmouth/books/sv/personal.phrasebook

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
	-rm -f $(kde_datadir)/kmouth/books/sv/.directory
	-rm -f $(kde_datadir)/kmouth/books/sv/courteousness.phrasebook
	-rm -f $(kde_datadir)/kmouth/books/sv/greetings.phrasebook
	-rm -f $(kde_datadir)/kmouth/books/sv/howareyou.phrasebook
	-rm -f $(kde_datadir)/kmouth/books/sv/personal.phrasebook
