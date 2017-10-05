RAGEL ?= ragel
BISON ?= bison

.SUFFIXES:
.PHONY: all clean

all: cc/lexer.cc \
	cc/grammars/typedruby24.cc \
	include/ruby_parser/diagnostic_class.hh \
	src/ffi_builder.rs \
	src/diagnostics.rs

clean:
	rm -f \
	cc/lexer.cc \
	cc/grammars/*.cc \
	cc/grammars/*.hh \
	include/ruby_parser/diagnostic_class.hh \
	src/ffi_builder.rs \
	src/diagnostics.rs

%.cc: %.rl
	$(RAGEL) -o $@ -C $<

%.cc %.hh: %.ypp
	$(BISON) --defines=$*.hh -o $*.cc $*.ypp

src/ffi_builder.rs: include/ruby_parser/builder.hh
	script/mkbuilder $< > $@

src/diagnostics.rs: script/mkdiagnostics
	script/mkdiagnostics rs > $@

include/ruby_parser/diagnostic_class.hh: script/mkdiagnostics
	script/mkdiagnostics cpp > $@
