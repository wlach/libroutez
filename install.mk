install-libroutez: libroutez.so
	$(INSTALL) -d $(DESTDIR)$(libdir)
	$(INSTALL_PROGRAM) libroutez.so $(DESTDIR)$(libdir)/ ;

# note: this is very non-idiomatic way of installing a python library. it
# probably doesn't handle edge cases well. but it works for me.
install-python: python/_tripgraph.so python/tripgraph.py
	$(INSTALL) -d $(DESTDIR)$(libdir)/python
	$(INSTALL) python/osm.py $(DESTDIR)$(libdir)/python
	$(INSTALL) python/_tripgraph.so $(DESTDIR)$(libdir)/python
	$(INSTALL) python/tripgraph.py $(DESTDIR)$(libdir)/python

install: install-libroutez install-python
