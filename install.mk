install-libroutez: libroutez.so
	$(INSTALL) -d $(DESTDIR)$(libdir)
	$(INSTALL_PROGRAM) libroutez.so $(DESTDIR)$(libdir)/ ;

# note: this is very non-idiomatic way of installing a python library. it
# probably doesn't handle edge cases well. but it works for me.
install-python: python/routez/_tripgraph.so python/routez/tripgraph.py
	$(INSTALL) -d $(DESTDIR)$(libdir)/python/routez
	$(INSTALL) python/routez/osm.py $(DESTDIR)$(libdir)/python/routez
	$(INSTALL) python/routez/_tripgraph.so $(DESTDIR)$(libdir)/python/routez
	$(INSTALL) python/routez/tripgraph.py $(DESTDIR)$(libdir)/python/routez

install: install-libroutez install-python
