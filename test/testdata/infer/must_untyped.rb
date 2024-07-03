# typed: strict

T.must(T.unsafe(nil))
#      ^^^^^^^^^^^^^ error: `T.must` called on `T.untyped`, which is redundant

T.must_because(T.unsafe(nil)) {'reason'}
#              ^^^^^^^^^^^^^ error: `T.must_because` called on `T.untyped`, which is redundant

anything = T.let(nil, T.anything)
T.must(anything) # error: `T.must` called on `T.anything`, which is redundant

object = T.let(nil, Object)
T.must(object) # error: `T.must` called on `Object`, which is redundant

basic_object = T.let(nil, BasicObject)
T.must(object) # error: `T.must` called on `BasicObject`, which is redundant
