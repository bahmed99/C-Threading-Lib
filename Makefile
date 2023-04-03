MAKEFLAGS += --no-print-directory

tests: test/*
	@for f in $^ ; do \
		out=$$(echo $$f | sed 's/\.c//g' | sed 's-test/-build/-g') ; make $$out ; \
	done

build/%: test/%.c build
	gcc -DUSE_PTHREAD -I . -o $@ $<

build:
	@mkdir -p build

.PHONY: tests