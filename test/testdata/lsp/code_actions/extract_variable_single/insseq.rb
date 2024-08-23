# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-variable: true

def and
  a = T.unsafe(1)
  b = T.unsafe(1)
  a && b
# ^ apply-code-action: [A] Extract Variable (this occurrence only)
#      ^ apply-code-action: [B] Extract Variable (this occurrence only)
end

def or
  a = T.unsafe(1)
  b = T.unsafe(1)
  a || b
# ^ apply-code-action: [C] Extract Variable (this occurrence only)
#      ^ apply-code-action: [D] Extract Variable (this occurrence only)
end
