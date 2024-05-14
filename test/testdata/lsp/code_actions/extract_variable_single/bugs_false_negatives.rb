# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-variable: true
# assert-no-code-action: refactor.extract

# This file contains cases where we should allow the user to extract to a variable,
# but incorrectly disallow it.

a = 1

case a
#    ^ apply-code-action: [A] Extract Variable
when Integer
end
