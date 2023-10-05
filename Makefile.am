AUTOMAKE_OPTIONS = gnu subdir-objects
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src etc $(DOC_DIR)
DISTDIRS = src etc $(DOC_DIR)

.PHONY: doc
doc:
	$(MAKE) -C doc doc

test:
	$(MAKE) -C src test
