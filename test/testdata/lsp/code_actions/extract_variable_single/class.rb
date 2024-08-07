# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-variable: true

class A; 1 + 123; end
#            ^^^ apply-code-action: [A] Extract Variable (this occurrence only)

class B; 1 + 1; 1 + 123; end
#                   ^^^ apply-code-action: [B] Extract Variable (this occurrence only)

class C
  1 + 123
#     ^^^ apply-code-action: [C] Extract Variable (this occurrence only)
end

class D
  1 + 1
  1 + 123
#     ^^^ apply-code-action: [D] Extract Variable (this occurrence only)
end

class E
  1 + 1
  1+ 2; 1 + 123
#           ^^^ apply-code-action: [E] Extract Variable (this occurrence only)
end
