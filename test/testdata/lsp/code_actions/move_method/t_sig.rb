# typed: strict
# selective-apply-code-action: refactor.extract

class Module
  include T::Sig
end

module Foo
  sig {void}
  def self.bar; end
         # | apply-code-action: [A] Move method to a new module
end
