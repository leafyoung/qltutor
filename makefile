
SUBDIRS=$(shell find . -maxdepth 1 ! -path . -type d)

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: clean

clean :
	for d in $(SUBDIRS); do ($(MAKE) clean -C $$d); done

