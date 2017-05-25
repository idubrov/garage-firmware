.PHONY: all
all: test

build/stepgen: stepgen-test.cpp ../src/algo/stepgen.cpp ../include/algo/stepgen.hpp
	mkdir -p `dirname $@`
	g++ -std=c++0x -I../include -o $@ stepgen-test.cpp ../src/algo/stepgen.cpp

test: build/stepgen
	mkdir -p build/testresult
	cat testdata | \
	while read CMD; do \
		build/stepgen $$CMD > "build/testresult/$${CMD// /_}" ; \
	done
	diff -u expected build/testresult/
