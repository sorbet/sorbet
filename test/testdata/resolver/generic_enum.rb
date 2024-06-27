# typed: true
# disable-fast-path: true
class MyEnum < T::Enum
  extend T::Generic
  Value = type_template {{fixed: String}} # error: All non-enum constants in a `T::Enum` must be defined after the `enums do` block

  enums do
    X = new # error: Type `Value` declared by parent `T.class_of(MyEnum)` must be re-declared
    Y = new # error: Type `Value` declared by parent `T.class_of(MyEnum)` must be re-declared
    Z = new # error: Type `Value` declared by parent `T.class_of(MyEnum)` must be re-declared
  end
end

extend T::Sig
sig {params(xy: T.any(MyEnum::X[Integer], MyEnum::Y[Integer])).void}
#                     ^^^^^^^^^^^^^^^^^^ error: Expected a class or module
#                              ^^^^^^^^^ error: Method `[]` does not exist
#                                         ^^^^^^^^^^^^^^^^^^ error: Expected a class or module
#                                                  ^^^^^^^^^ error: Method `[]` does not exist
def foo(xy)
end

XY = T.type_alias {T.any(MyEnum::X[Integer], MyEnum::Y[Integer])}
#                        ^^^^^^^^^^^^^^^^^^ error: Expected a class or module
#                                 ^^^^^^^^^ error: Method `[]` does not exist
#                                            ^^^^^^^^^^^^^^^^^^ error: Expected a class or module
#                                                     ^^^^^^^^^ error: Method `[]` does not exist
