# typed: true
# no-stdlib: true
  ::B=Struct.new:x
# ^^^^^^^^^^^^^^^^ error: Method `unsafe` does not exist on `T.class_of(T)`
# ^^^^^^^^^^^^^^^^ error: Method `type_member` does not exist on `T.class_of(B)`
# ^^^^^^^^^^^^^^^^ error: Method `sig` does not exist on `T.class_of(T::Sig::WithoutRuntime)`
# ^^^^^^^^^^^^^^^^ error: Method `params` does not exist on `T.class_of(B)`
