# typed: strict

T.must(T.unsafe(nil))
#      ^^^^^^^^^^^^^ error: `T.must` called on `T.untyped`, which is redundant

T.must_because(T.unsafe(nil)) {'reason'}
#              ^^^^^^^^^^^^^ error: `T.must_because` called on `T.untyped`, which is redundant