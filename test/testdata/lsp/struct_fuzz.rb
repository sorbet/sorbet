# typed: true
# no-stdlib: true
  ::B=Struct.new:x
# ^^^^^^^^^^^^^^^^ error: Method `keep_def` does not exist on `T.class_of(Sorbet::Private::Static)`
# ^^^^^^^^^^^^^^^^ error: Method `type_member` does not exist on `T.class_of(B)`
# ^^^^^^^^^^^^^^^^ error: Method `params` does not exist on `T.class_of(B)`
# ^^^^^^^^^^^^^^^^ error: Method `unsafe` does not exist on `T.class_of(T)`
# ^^^^^^^^^^^^^^^^ error: Method `unsafe` does not exist on `T.class_of(T)`
# ^^^^^^^^^^^^^^^^ error: Method `unsafe` does not exist on `T.class_of(T)`
#                ^ error: Method `keep_def` does not exist on `T.class_of(Sorbet::Private::Static)`
#                ^ error: Method `keep_def` does not exist on `T.class_of(Sorbet::Private::Static)`
# these errors aren't actually what we're checking for, we just want to make sure sorbet doesn't crash on this input
# when no-stdlib is true.
