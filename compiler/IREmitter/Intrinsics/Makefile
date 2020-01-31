
all: intrinsics-report.md ../WrappedIntrinsics.h ../Payload/PayloadIntrinsics.c ../../../third_party/ruby/export-intrinsics.patch

../WrappedIntrinsics.h: WrappedIntrinsics.formatted.h
	cp $< $@

../Payload/PayloadIntrinsics.c: PayloadIntrinsics.formatted.c
	cp $< $@

../../../third_party/ruby/export-intrinsics.patch: export-intrinsics.patch
	cp $< $@

%.formatted.c: %.c
	bazel run @com_stripe_ruby_typer//tools:clang-format "$(PWD)/$<" > "$(PWD)/$@"
%.formatted.h: %.h
	bazel run @com_stripe_ruby_typer//tools:clang-format "$(PWD)/$<" > "$(PWD)/$@"

../../../bazel-bin/external/sorbet_ruby_unpatched/toolchain/bin/ruby:
	bazel build @sorbet_ruby_unpatched//:ruby -c opt

intrinsics-report.md WrappedIntrinsics.h PayloadIntrinsics.c export-intrinsics.patch: wrap-intrinsics.rb ../../../bazel-bin/external/sorbet_ruby_unpatched/toolchain/bin/ruby
	../../../bazel-bin/external/sorbet_ruby_unpatched/toolchain/bin/ruby wrap-intrinsics.rb

clean:
	$(RM) WrappedIntrinsics.h WrappedIntrinsics.formatted.h
	$(RM) PayloadIntrinsics.c PayloadIntrinsics.formatted.c
	$(RM) export-intrinsics.patch
