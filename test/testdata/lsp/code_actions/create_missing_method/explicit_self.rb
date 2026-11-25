# typed: true
# selective-apply-code-action: refactor
# enable-experimental-lsp-create-missing-method: true

class ExplicitSelf
  extend T::Sig

  sig { void }
  def caller
    self.do_something(42)
#        ^^^^^^^^^^^^ error: Method `do_something` does not exist on `ExplicitSelf`
#          ^ apply-code-action: [A] Create missing method
  end
end
