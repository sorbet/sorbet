# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-variable: true

def csend
  a = T.let(1, T.nilable(Integer))
  puts(a&.to_s)
#      ^ apply-code-action: [A] Extract Variable (this occurrence only)
end

def and
  a = T.unsafe(1)
  b = T.unsafe(1)
  a && b
# ^ TODO: [B] Extract Variable (this occurrence only)
#      ^ apply-code-action: [C] Extract Variable (this occurrence only)
end

def or
  a = T.unsafe(1)
  b = T.unsafe(1)
  a || b
# ^ TODO: [D] Extract Variable (this occurrence only)
#      ^ apply-code-action: [E] Extract Variable (this occurrence only)
end

