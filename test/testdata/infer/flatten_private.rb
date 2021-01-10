# typed: true

class A
  extend T::Sig

  # Horrible incompatabible override but I think it's fine.
  # The purpose here is to prove (without needing to scruitinze the exp output)
  # that we're leaving behind the symbol in the tree.
  sig {params(arg0: Integer).void}
  def self.private(arg0); end
  sig {params(arg0: Integer).void}
  def self.private_class_method(arg0); end

  private_class_method def self.outer_static # error: Expected `Integer` but found `Symbol(:outer_static)` for argument `arg0`
    private_class_method def self.inner_static; end # error: Expected `Integer` but found `Symbol(:inner_static)` for argument `arg0`
  end

  private def outer_instance # error: Expected `Integer` but found `Symbol(:outer_instance)` for argument `arg0`
    self.class.private def inner_instance; end # error: Expected `Integer` but found `Symbol(:inner_instance)` for argument `arg0`
  end
end
