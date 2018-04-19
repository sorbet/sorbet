# typed: strict
class TestGeneric
  extend T::Generic
  Elem = type_member

  def foo
    # TODO(nelhage): Should we allow this? This test is here because
    # we used to crash and we shouldn't do that, but implementing this
    # at runtime faithfully requires non-erasure.
    T::Array[Elem].new
  end
end
