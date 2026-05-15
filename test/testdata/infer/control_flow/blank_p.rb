# typed: true

class FalseClass
  extend T::Sig

  sig {returns(TrueClass)}
  def blank?
    true
  end
end

class NilClass
  extend T::Sig

  sig {returns(TrueClass)}
  def blank?
    true
  end
end

class Object
  def blank?
    !!self
  end
end

class Time
  # No Time is blank
  T::Sig::WithoutRuntime.sig { override.returns(FalseClass) }
  def blank? = false
end

class NeverPresent
  T::Sig::WithoutRuntime.sig { returns(TrueClass) }
  def blank? = true
end

class A
  extend T::Sig

  sig {params(s: T.nilable(String)).returns(NilClass)}
  def test_return(s)
    if !s.blank?
      s.length
      nil
    end
    nil
  end

  sig {params(s: String).returns(NilClass)}
  def test_unchanged(s)
    if s.blank?
      s.length
    else
      s.length
    end
    nil
  end

  def unreachable_nil()
    a = nil
    if !a.blank?
      puts a # error: This code is unreachable
    end
  end

  def unreachable_false()
    a = false
    if !a.blank?
      puts a # error: This code is unreachable
    end
  end

  sig { params(time: T.nilable(Time)).void }
  def test_never_blank(time)
    if time.blank?
      T.reveal_type(time) # error: Revealed type: `NilClass`
    else
      T.reveal_type(time) # error: Revealed type: `Time`
    end
  end

  sig {
    type_parameters(:U)
      .params(time: T.nilable(T.all(T.type_parameter(:U), Time)))
      .void
  }
  def test_not_fully_defined(time)
    if time.blank?
      T.reveal_type(time) # error: Revealed type: `T.nilable(T.all(Time, T.type_parameter(:U) (of A#test_not_fully_defined)))`
    else
      T.reveal_type(time) # error: Revealed type: `T.all(Time, T.type_parameter(:U) (of A#test_not_fully_defined))`
    end
  end

  sig { params(x: T.any(String, NeverPresent)).void }
  def test_custom_never_blank(x)
    if x.blank?
      T.reveal_type(x) # error: `T.any(String, NeverPresent)`
    else
      T.reveal_type(x) # error: `T.any(String, NeverPresent)`
    end
  end
end
