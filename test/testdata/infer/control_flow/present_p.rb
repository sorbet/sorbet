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
  extend T::Sig

  sig {returns(TrueClass)}
  def present?
    true
  end
end

TimeOrNil = T.type_alias { T.nilable(Time) }

class Widget; end

class Box
  extend T::Sig
  extend T::Generic

  Elem = type_member

  sig {returns(Elem)}
  def present?
    T.unsafe(nil)
  end
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

  sig {params(time: T.nilable(Time)).returns(NilClass)}
  def test_falsy_branch_narrows(time)
    if time.present?
      T.reveal_type(time) # error: Revealed type: `Time`
    else
      T.reveal_type(time) # error: Revealed type: `NilClass`
    end
    nil
  end

  sig {params(time: TimeOrNil).returns(NilClass)}
  def test_falsy_branch_narrows_alias(time)
    if time.present?
      T.reveal_type(time) # error: Revealed type: `Time`
    else
      T.reveal_type(time) # error: Revealed type: `NilClass`
    end
    nil
  end

  sig {params(w: T.nilable(Widget)).returns(NilClass)}
  def test_falsy_branch_no_narrow_without_sig(w)
    # Widget inherits Object#present? which has no sig, so we
    # conservatively do not narrow away Widget in the falsy branch
    if !w.present?
      T.reveal_type(w) # error: Revealed type: `T.nilable(Widget)`
    end
    nil
  end

  sig {params(x: T.any(Box[NilClass], Time)).returns(NilClass)}
  def test_falsy_branch_narrows_instantiated_sig(x)
    if !x.present?
      T.reveal_type(x) # error: Revealed type: `Box[NilClass]`
    end
    nil
  end
end
