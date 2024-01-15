# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-variable: true
#
# This file contain cases where we should disallow the user from extracting to variable,
# but incorrectly allow it.

/W[aeiou]rd/i
#           ^ apply-code-action: [A] Extract Variable

  /W[aeiou]rd/i
#  ^^^^^^^^^^ apply-code-action: [B] Extract Variable

class A < T::Struct
  prop :x, String
#      ^^ apply-code-action: [C] Extract Variable
end
