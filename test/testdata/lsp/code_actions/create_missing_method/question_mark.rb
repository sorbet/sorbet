# typed: true
# selective-apply-code-action: refactor
# enable-experimental-lsp-create-missing-method: true

class QuestionMark
  extend T::Sig

  sig { void }
  def caller
    valid?(42)
#   ^^^^^^ error: Method `valid?` does not exist on `QuestionMark`
#     ^ apply-code-action: [A] Create missing method
  end
end
