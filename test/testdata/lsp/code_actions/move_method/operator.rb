# typed: strict
# selective-apply-code-action: refactor.extract
# assert-no-code-action: refactor.extract
#
# The refactoring should not be available for the operator overloads

module Foo
  extend T::Sig
  sig {params(obj: T.untyped).returns(T::Boolean)}
  def self.<=(obj)
     # | apply-code-action: [A] Move method to a new module
    false
  end
end

Foo == Foo

