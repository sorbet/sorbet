# typed: true
# selective-apply-code-action: refactor.rewrite
# assert-no-code-action: refactor.rewrite
extend T::Sig

class A < T::Struct
  extend T::Sig

  attr_reader :foo
  #            ^ apply-code-action: [A] Convert to singleton class method (best effort)

  prop :foo, Integer
  #     ^ apply-code-action: [A] Convert to singleton class method (best effort)
end
