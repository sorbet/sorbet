# typed: true
# selective-apply-code-action: refactor
# enable-experimental-lsp-create-missing-method: true

class VariedLiterals
  extend T::Sig

  sig { void }
  def caller
    process(nil, :my_symbol, 3.14, [1, 2, 3], {a: 1})
#   ^^^^^^^ error: Method `process` does not exist on `VariedLiterals`
#     ^ apply-code-action: [A] Create missing method
  end
end
