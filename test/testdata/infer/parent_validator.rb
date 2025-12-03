# typed: strict

class Parent; end

class FinalClass
  extend T::Helpers
  final!
end

# This is similar to infer/t_module_validator.rb, but this types the @mod as a
# type member, instead of as an AppliedType passed a type member
class ParentValidator
  extend T::Sig, T::Generic
  abstract!

  ModParent = type_member { {upper: T::Module[Parent]} }
  ClassOfFinal = type_member { {upper: T.class_of(FinalClass) } }

  sig { params(mod_parent: ModParent, class_of_final: ClassOfFinal).void }
  def initialize(mod_parent, class_of_final)
    @mod_parent = mod_parent
    @class_of_final = class_of_final
  end

  sig { params(other: T.anything).returns(T.nilable(Parent)) }
  def downcast1(other)
    T.reveal_type(other) # error: `T.anything`
    case other
    when @mod_parent
      T.reveal_type(other) # error: `T.anything`
      return other # error: Expected `T.nilable(Parent)` but found `T.anything` for method result type
    else
      T.reveal_type(other) # error: `T.anything`
      return nil
    end
  end

  sig { params(other: T.anything).returns(T.nilable(FinalClass)) }
  def downcast2(other)
    T.reveal_type(other) # error: `T.anything`
    case other
    when @class_of_final
      T.reveal_type(other) # error: `T.anything`
      return other # error: Expected `T.nilable(FinalClass)` but found `T.anything` for method result type
    else
      T.reveal_type(other) # error: `T.anything`
      return nil
    end
  end

  sig { params(x: T.any(FinalClass, Integer)).void }
  def example3(x)
    T.reveal_type(x) # error: `T.any(FinalClass, Integer)`
    case x
    when @class_of_final
      T.reveal_type(x) # error: `T.any(FinalClass, Integer)`
    when Integer
      T.reveal_type(x) # error: `Integer`
    else
      T.absurd(x) # error: Control flow could reach `T.absurd` because the type `FinalClass` wasn't handled
    end
  end
end
