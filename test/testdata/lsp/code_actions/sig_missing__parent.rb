# typed: strict
# selective-apply-code-action: quickfix

class Foo
  extend T::Sig

  sig {overridable.void}
  def self.bar
  end
end
