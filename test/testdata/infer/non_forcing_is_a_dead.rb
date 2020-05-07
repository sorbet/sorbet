# typed: true

class MyStruct
  sig {returns(Integer)}
  def foo
    # We know this check is false, but we can't say so, because similar checks
    # get included in generated code before we can know that the generated check
    # will fail.
    if T::NonForcingConstants.non_forcing_is_a?(self, '::Integer')
      puts self
    end

    0
  end
end
