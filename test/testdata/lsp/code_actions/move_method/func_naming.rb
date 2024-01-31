# typed: strict
# selective-apply-code-action: refactor.extract
#
# The test asserts the refactoring will select a unique name
# for a new module based on a method name
# In this case the new module name should `GreetingModule1`,
# because `GreetingModule` is already taken

module GreetingModule; end
module Foo
  extend T::Sig

  sig {returns(String)}
  def self._greeting_
         # | apply-code-action: [A] Move method to a new module
    'Hello'
  end
end
