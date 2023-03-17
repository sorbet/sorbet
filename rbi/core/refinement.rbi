# typed: __STDLIB_INTERNAL

# [Refinement](https://docs.ruby-lang.org/en/3.2/Refinement.html) is a class of
# the self (current context) inside refine statement. It allows to import
# methods from other modules, see
# [import_methods](https://docs.ruby-lang.org/en/3.2/Refinement.html#method-i-import_methods).
class Refinement < Object
  # Imports methods from modules. Unlike
  # [Module#include](https://docs.ruby-lang.org/en/3.2/Module.html#method-i-include),
  # [Refinement#import_methods](https://docs.ruby-lang.org/en/3.2/Refinement.html#method-i-import_methods)
  # copies methods and adds them into the refinement, so the refinement is
  # activated in the imported methods.
  #
  # Note that due to method copying, only methods defined in Ruby code can be imported.
  #
  # ```ruby
  # module StrUtils
  #   def indent(level)
  #     ' ' * level + self
  #   end
  # end
  #
  # module M
  #   refine String do
  #     import_methods StrUtils
  #   end
  # end
  #
  # using M
  # "foo".indent(3)
  # #=> "   foo"
  #
  # module M
  #   refine String do
  #     import_methods Enumerable
  #     # Can't import method which is not defined with Ruby code: Enumerable#drop
  #   end
  # end
  # ```
  #
  # Also aliased as: import_methods
  sig do
    params(
      mod: Module,
      rest: Module,
    )
    .returns(Refinement)
  end
  def import_methods(mod, *rest); end

  # Return the class refined by the receiver.
  sig {returns(T.class_of(Object))}
  def refined_class; end
end
