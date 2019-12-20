
all: intrinsic-report.md ../WrappedIntrinsics.h ../Payload/PayloadIntrinsics.c

../WrappedIntrinsics.h: WrappedIntrinsics.h-formatted
	cp $< $@

../Payload/PayloadIntrinsics.c: PayloadIntrinsics.c-formatted
	cp $< $@

%-formatted: %
	bazel run @com_stripe_ruby_typer//tools:clang-format "$(PWD)/$<" > "$(PWD)/$@"

intrinsics-report.md WrappedIntrinsics.h PayloadIntrinsics.c: wrap-intrinsics.rb
	./wrap-intrinsics.rb

clean:
	$(RM) WrappedIntrinsics.h WrappedIntrinsics.h-formatted
	$(RM) PayloadIntrinsics.c PayloadIntrinsics.c-formatted
