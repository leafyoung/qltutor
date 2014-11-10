
SUBDIRS=$(shell find . -maxdepth 1 ! -path . -and ! -path './.git' -type d)

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: clean

clean :
	for d in $(SUBDIRS); do ($(MAKE) clean -C $$d); done

test :
	for d in $(SUBDIRS); do ($(MAKE) test -C $$d); done


