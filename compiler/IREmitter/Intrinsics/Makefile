
all: intrinsic-report.md ../WrappedIntrinsics.h ../Payload/PayloadIntrinsics.c

../WrappedIntrinsics.h: WrappedIntrinsics.h
	cp $< $@

../Payload/PayloadIntrinsics.c: PayloadIntrinsics.c
	cp $< $@

intrinsics-report.md WrappedIntrinsics.h PayloadIntrinsics.c: wrap-intrinsics.rb
	./wrap-intrinsics.rb
