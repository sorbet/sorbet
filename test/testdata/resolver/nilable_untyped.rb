# typed: false


Bad = T.type_alias {T.nilable(T.untyped)}
#                   ^^^^^^^^^^^^^^^^^^^^ error: `T.nilable(T.untyped)` is the same as `T.untyped`

# This is a tough to handle case, as the argument to `T.nilable` in `Tricky` is
# still `T.untyped`, but it's hard to distinguish that from a case where
# `T.untyped` was returned because the argument type failed to parse in
# type_syntax.cc. The compromise here is to only raise the error when we
# encounter `T.nilable(T.untyped)` explicitly.
Untyped = T.type_alias {T.untyped}
Tricky = T.type_alias {T.nilable(Untyped)}

class A
  extend T::Sig
  extend T::Generic

  X = type_member {{upper: T.nilable(T.untyped)}}
  #                        ^^^^^^^^^^^^^^^^^^^^ error: `T.nilable(T.untyped)` is the same as `T.untyped`

  sig {params(x: T.nilable(T.untyped)).void}
  #              ^^^^^^^^^^^^^^^^^^^^ error: `T.nilable(T.untyped)` is the same as `T.untyped`
  def test1(x)
  end

end

class B < T::Struct

  prop :foo, T.nilable(T.untyped)
  #          ^^^^^^^^^^^^^^^^^^^^ error: `T.nilable(T.untyped)` is the same as `T.untyped`

  const :bar, T.nilable(T.untyped), default: 10
  #           ^^^^^^^^^^^^^^^^^^^^ error: `T.nilable(T.untyped)` is the same as `T.untyped`
end
