ifdef WIN32
	OPTIONS += WIN32=1
else
ifdef DOS
	OPTIONS += DOS=1
endif
endif

all install uninstall clean:
	$(MAKE) -C src $(OPTIONS) $@
