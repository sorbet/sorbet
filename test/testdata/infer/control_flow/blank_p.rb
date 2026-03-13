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
  extend T::Sig

  sig {returns(FalseClass)}
  def blank?
    false
  end
end

TimeOrNil = T.type_alias { T.nilable(Time) }

class Widget; end

class Box
  extend T::Sig
  extend T::Generic

  Elem = type_member

  sig {returns(Elem)}
  def blank?
    T.unsafe(nil)
  end
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

  sig {params(time: T.nilable(Time)).returns(NilClass)}
  def test_truthy_branch_narrows(time)
    if time.blank?
      T.reveal_type(time) # error: Revealed type: `NilClass`
    else
      T.reveal_type(time) # error: Revealed type: `Time`
    end
    nil
  end

  sig {params(time: TimeOrNil).returns(NilClass)}
  def test_truthy_branch_narrows_alias(time)
    if time.blank?
      T.reveal_type(time) # error: Revealed type: `NilClass`
    else
      T.reveal_type(time) # error: Revealed type: `Time`
    end
    nil
  end

  sig {params(w: T.nilable(Widget)).returns(NilClass)}
  def test_truthy_branch_no_narrow_without_sig(w)
    # Widget inherits Object#blank? which has no sig, so we
    # conservatively do not narrow away Widget in the truthy branch
    if w.blank?
      T.reveal_type(w) # error: Revealed type: `T.nilable(Widget)`
    end
    nil
  end

  sig {params(x: T.any(Box[TrueClass], Time)).returns(NilClass)}
  def test_truthy_branch_narrows_instantiated_sig(x)
    if x.blank?
      T.reveal_type(x) # error: Revealed type: `Box[TrueClass]`
    end
    nil
  end
end
