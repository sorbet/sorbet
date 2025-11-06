# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-variable: true

a = "a" + "b"
#   ^^^ apply-code-action: [A] Extract Variable (this occurrence only)

b = :abc
