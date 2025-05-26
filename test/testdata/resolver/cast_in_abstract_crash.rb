# typed: true
class A
  extend T::Sig
  extend T::Helpers

  abstract!

  sig {abstract.void}
  def foo
    @z ||= T.let(nil, T.nilable(T.type_parameter(:U)))
#   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Abstract methods must not contain any code in their body
#          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve declared type for `@z`
#                                                ^^ error: Unspecified type parameter
  end
end
