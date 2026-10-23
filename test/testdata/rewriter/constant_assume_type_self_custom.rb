# typed: true

class NilableNew
  extend T::Sig

  sig {returns(T.nilable(T.attached_class))}
  def self.new
    return if [true, false].sample
    super
  end

  INSTANCE = new # error: Assumed expression had type `NilableNew` but found `T.nilable(T.attached_class (of NilableNew))`
end

T.reveal_type(NilableNew::INSTANCE) # error: `NilableNew`

class GenericNew
  extend T::Generic

  Elem = type_member
  INSTANCE = new
end

T.reveal_type(GenericNew::INSTANCE) # error: `T.untyped`

class FixedNew
  extend T::Generic

  Elem = type_member {{fixed: Integer}}
  INSTANCE = new
end

T.reveal_type(FixedNew::INSTANCE) # error: `FixedNew`

module ModuleNew
  extend T::Sig

  sig {returns(Integer)}
  def self.new
    0
  end

  INSTANCE = new
end

T.reveal_type(ModuleNew::INSTANCE) # error: `T.untyped`

# At the top level, self is an instance of Object, not a class.
extend T::Sig

sig {returns(Integer)}
def new
  0
end

TOP_LEVEL = new
T.reveal_type(TOP_LEVEL) # error: `T.untyped`
