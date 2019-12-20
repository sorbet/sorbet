
# Ruby intrinsic analyzer

## Prerequisites

* ruby binary
* ruby source

## Running

Running `make` will generate the following files:

- `intrinsic-report.md` which details all of the intrinsics that are exported
  from the ruby located in `//bazel-bin/external/ruby_2_6_3/bin/ruby`, versus
  all of the intrinsics that are defined with `rb_define_method` in the ruby
  source in `~/Downloads/ruby-2.6.3`.
- `PayloadIntrinsics.c` includes intrinsic definitions for all intrinsics that
  are exposed on the ruby executable we're linking with.
- `WrappedIntrinsics.h` is a c++ vector initializer fragment that's meant to be
  included into `SymbolBasedIntrinsics.cc`, and is what connects the wrappers
  defined in `PayloadIntrinsics.c`.
