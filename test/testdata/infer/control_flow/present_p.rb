# typed: true

class FalseClass
  extend T::Sig

  sig {returns(FalseClass)}
  def present?
    false
  end
end

class NilClass
  extend T::Sig

  sig {returns(FalseClass)}
  def present?
    false
  end
end

class Object
  def present?
    !self
  end
end

class Time
  # Time is always present, because it is never blank
  T::Sig::WithoutRuntime.sig { returns(TrueClass) }
  def present? = true
end

class NeverPresent
  T::Sig::WithoutRuntime.sig { returns(FalseClass) }
  def present? = false
end

class A
  extend T::Sig

  sig {params(s: T.nilable(String)).returns(NilClass)}
  def test_return(s)
    if s.present?
      s.length
      nil
    end
    nil
  end

  sig {params(s: String).returns(NilClass)}
  def test_unchanged(s)
    if s.present?
      s.length
    else
      s.length
    end
    nil
  end

  def unreachable_nil()
    if nil.present?
      "foo" # error: This code is unreachable
    end
  end

  def unreachable_false()
    if false.present?
      "foo" # error: This code is unreachable
    end
  end

  sig { params(time: T.nilable(Time)).void }
  def test_always_present(time)
    if time.present?
      T.reveal_type(time) # error: Revealed type: `Time`
    else
      T.reveal_type(time) # error: Revealed type: `T.nilable(Time)`
    end
  end

  sig {
    type_parameters(:U)
      .params(time: T.nilable(T.all(T.type_parameter(:U), Time)))
      .void
  }
  def test_always_present_not_fully_defined(time)
    if time.present?
      T.reveal_type(time) # error: Revealed type: `T.all(Time, T.type_parameter(:U) (of A#test_always_present_not_fully_defined))`
    else
      T.reveal_type(time) # error: Revealed type: `T.nilable(T.all(Time, T.type_parameter(:U) (of A#test_always_present_not_fully_defined)))`
    end
  end

  sig { params(x: T.any(String, NeverPresent)).void }
  def test_custom_never_present(x)
    if x.present?
      T.reveal_type(x) # error: `T.any(String, NeverPresent)`
    else
      T.reveal_type(x) # error: `T.any(String, NeverPresent)`
    end
  end
end
