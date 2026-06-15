# typed: true
# selective-apply-code-action: refactor
# enable-experimental-lsp-extract-to-variable: true

extend T::Sig

sig { void }
def caller
  do_something("hello")
# ^^^^^^^^^^^^ error: Method `do_something` does not exist on `Object`
#   ^ apply-code-action: [A] Create missing method
end
