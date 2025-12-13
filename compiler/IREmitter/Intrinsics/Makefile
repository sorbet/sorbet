TOPDIR := ../../..
BAZEL := $(TOPDIR)/bazel

all: intrinsics-report.md ../WrappedIntrinsics.h ../Payload/PayloadIntrinsics.c

../WrappedIntrinsics.h: WrappedIntrinsics.formatted.h
	cp $< $@

../Payload/PayloadIntrinsics.c: PayloadIntrinsics.formatted.c
	cp $< $@

%.formatted.c: %.c
	$(BAZEL) run @com_stripe_ruby_typer//tools:clang-format "$(PWD)/$<" > "$(PWD)/$@"
%.formatted.h: %.h
	$(BAZEL) run @com_stripe_ruby_typer//tools:clang-format "$(PWD)/$<" > "$(PWD)/$@"

ensure-ruby:
	$(BAZEL) build @sorbet_ruby_2_7_unpatched//:ruby

write-files: wrap-intrinsics.rb ensure-ruby
	$(TOPDIR)/bazel-bin/external/sorbet_ruby_2_7_unpatched/toolchain/bin/ruby wrap-intrinsics.rb

intrinsics-report.md WrappedIntrinsics.h PayloadIntrinsics.c: write-files

clean:
	$(RM) WrappedIntrinsics.h WrappedIntrinsics.formatted.h
	$(RM) PayloadIntrinsics.c PayloadIntrinsics.formatted.c
