# typed: strict
# selective-apply-code-action: refactor.extract

module Bar
  extend T::Sig

  sig {void}
  def self.bar; end
         # | apply-code-action: [A] Move method to a new module
end
