# typed: false

class NormalClass
end

class NewIsNilable
  extend T::Sig

  sig {returns(T.nilable(T.attached_class))}
  def self.new
    return if [true, false].sample
    super
  end
end

class NewIsSpecific
  extend T::Sig

  sig {returns(NewIsSpecificChild)}
  def self.new
    NewIsSpecificChild.new
  end
end
class NewIsSpecificChild < NewIsSpecific
end

A = NormalClass.new
B = NewIsNilable.new
C1 = NewIsSpecific.new
C2 = T.let(NewIsSpecific.new, NewIsSpecific)
C3 = NewIsSpecificChild.new
