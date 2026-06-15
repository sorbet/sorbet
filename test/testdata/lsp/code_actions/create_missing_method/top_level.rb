# typed: true
# selective-apply-code-action: refactor
# enable-experimental-lsp-extract-to-variable: true
# assert-no-code-action: refactor

do_something("hello") # error: Method `do_something` does not exist on `T.class_of(<root>)`
