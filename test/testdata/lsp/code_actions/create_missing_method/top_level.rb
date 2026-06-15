# typed: true
# selective-apply-code-action: refactor
# enable-experimental-lsp-create-missing-method: true
# assert-no-code-action: refactor

do_something("hello") # error: Method `do_something` does not exist on `T.class_of(<root>)`
