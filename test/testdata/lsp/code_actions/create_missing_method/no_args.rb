# typed: true
# selective-apply-code-action: refactor
# enable-experimental-lsp-extract-to-variable: true

class NoArgs
  extend T::Sig

  sig { void }
  def caller
    do_something
#   ^^^^^^^^^^^^ error: Method `do_something` does not exist on `NoArgs`
#     ^ apply-code-action: [A] Create missing method
  end
end
