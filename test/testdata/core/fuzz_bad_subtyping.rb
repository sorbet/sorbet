# typed: true
# disable-fast-path: true
# Found by fuzzer: https://github.com/sorbet/sorbet/issues/1132
module MyEnumerable
  extend T::Generic
  A = type_member
  sig {params(a: MyEnumerable[])}
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Malformed `sig`: No return type specified. Specify one with .returns()
  #                          ^^ error: Wrong number of type parameters for `MyEnumerable`.
  #                          ^^ error: Wrong number of type parameters for `MyEnumerable`.
  #    ^^^^^^ error: Method `params` does not exist on `T.class_of(MyEnumerable)`
# ^^^ error: Method `sig` does not exist on `T.class_of(MyEnumerable)`
  def -(a) end
  class MySet
    include MyEnumerable
    A = # error: Type variable `A` needs to be declared as a type_member or type_template, not a static-field
      new - MySet.new()
  end
end
