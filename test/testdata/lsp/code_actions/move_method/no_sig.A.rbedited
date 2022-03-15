# typed: strict
# selective-apply-code-action: refactor.extract
#
# The test is asserting we can't move a method without a sig

module Foo
  extend T::Sig

  def self.bar
# ^^^^^^^^^^^^ error: This function does not have a `sig`
         # ^^^ apply-code-action: [A] Move method to a new module
    'Hello'
  end
end

