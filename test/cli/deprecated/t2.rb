# typed: true

# Basic deprecated method
class BasicDeprecated
  extend T::Sig
  
  sig { deprecated.returns(String) }
  def old_method
    "legacy"
  end
  
  sig { returns(String) }
  def new_method
    "current"
  end
end

