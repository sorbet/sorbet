# typed: strict

T.must(T.unsafe(nil))
#      ^^^^^^^^^^^^^ error: `T.must` called on `T.untyped`, which is redundant
