# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-variable: true

  /abc/i.match("abcd")
# ^^^^^^ apply-code-action: [A] Extract Variable (this occurrence only)

Regexp.new("abc").match("abcd")
#          ^^^^^ apply-code-action: [B] Extract Variable (this occurrence only)
Regexp.new("xyz", "m").match("xyz1")
#                 ^^^ apply-code-action: [C] Extract Variable (this occurrence only)
