# typed: true
# enable-experimental-rbs-comments: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-variable: true

#: () -> Integer
def total
  1 + 2
#     ^ apply-code-action: [A] Extract Variable (this occurrence only)
end
