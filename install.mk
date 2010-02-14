install-libroutez: libroutez.so
	$(INSTALL) -d $(DESTDIR)$(libdir)
	$(INSTALL_PROGRAM) libroutez.so $(DESTDIR)$(libdir)/ ;

# note: this is very non-idiomatic way of installing a python library. it
# probably doesn't handle edge cases well. but it works for me.
install-python: python/libroutez/_tripgraph.so python/libroutez/tripgraph.py
	$(INSTALL) -d $(DESTDIR)$(libdir)/python/libroutez
	$(INSTALL) python/libroutez/osm.py $(DESTDIR)$(libdir)/python/libroutez
	$(INSTALL) python/libroutez/_tripgraph.so $(DESTDIR)$(libdir)/python/libroutez
	$(INSTALL) python/libroutez/tripgraph.py $(DESTDIR)$(libdir)/python/libroutez
	$(INSTALL) python/libroutez/__init__.py $(DESTDIR)$(libdir)/python/libroutez

# likewise, this is a very non idiomatic way of installing a ruby module...
install-ruby:
	$(INSTALL) -d $(DESTDIR)$(libdir)/ruby
	$(INSTALL) ruby/routez.so $(DESTDIR)$(libdir)/ruby

install-util:
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL) utils/creategraph.py $(DESTDIR)$(bindir)
	$(INSTALL) utils/get-gtfs-bounds.py $(DESTDIR)$(bindir)


install: install-libroutez install-python install-util install-ruby
