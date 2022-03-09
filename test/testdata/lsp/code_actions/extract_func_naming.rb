# typed: strict
# selective-apply-code-action: refactor.extract

module GreetingModule; end
module Foo
  extend T::Sig

  sig {returns(String)}
  def self._greeting_
         # ^^^^^^^^^^ apply-code-action: [A] Extract method to module
    'Hello'
  end
end
