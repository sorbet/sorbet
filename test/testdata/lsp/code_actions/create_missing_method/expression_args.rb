# typed: true
# selective-apply-code-action: refactor
# enable-experimental-lsp-extract-to-variable: true

class ExpressionArgs
  extend T::Sig

  sig { params(x: Integer, y: Integer).void }
  def caller(x, y)
    compute(x + y, x.to_s, [x, y].length)
#   ^^^^^^^ error: Method `compute` does not exist on `ExpressionArgs`
#     ^ apply-code-action: [A] Create missing method
  end
end
