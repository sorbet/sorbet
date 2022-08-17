# typed: strict

class A
  extend T::Sig

  @@foo = T.let(nil, T.nilable(Foo))
  #                            ^^^ error: Unable to resolve constant
  @@bar = T.let(Bar.new, Bar)
  #             ^^^ error: Unable to resolve constant
  #                      ^^^ error: Unable to resolve constant
  @@baz = T.let(Baz1.new, T.any(Baz1, Baz2))
  #             ^^^^ error: Unable to resolve constant
  #                             ^^^^ error: Unable to resolve constant
  #                                   ^^^^ error: Unable to resolve constant
  @@biz = T.let(nil, T.nilable(T.type_parameter(:U)))
  #       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve declared type
  #                                             ^^ error: Unspecified type parameter
  sig {void}
  def initialize
    @foo = T.let(nil, T.nilable(Foo))
    #                           ^^^ error: Unable to resolve constant
    @bar = T.let(Bar.new, Bar)
    #            ^^^ error: Unable to resolve constant
    #                     ^^^ error: Unable to resolve constant
    @baz = T.let(Baz1.new, T.any(Baz1, Baz2))
    #            ^^^^ error: Unable to resolve constant
    #                            ^^^^ error: Unable to resolve constant
    #                                  ^^^^ error: Unable to resolve constant
    @biz = T.let(nil, T.nilable(T.type_parameter(:U)))
    #      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve declared type
    #                           ^^^^^^^^^^^^^^^^^^^^ error: Method `A#initialize` does not declare any type parameters
    #                                            ^^ error: Unspecified type parameter
  end
end
