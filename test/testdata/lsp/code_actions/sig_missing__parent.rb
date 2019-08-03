# typed: strict
# exhaustive-apply-code-action: true

class Foo
  extend T::Sig

  sig {implementation.void}
  def self.bar
  end
end
