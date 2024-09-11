# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-variable: true

class A
  def initialize(x)
  end
end

def test
  temp = T.unsafe(1)
  case temp
  when nil
  when A
    temp
#   ^^^^ apply-code-action: [A] Extract Variable (this occurrence only)
#   ^^^^ apply-code-action: [B] Extract Variable (all 3 occurrences)
  when Integer
    A.new(temp)
#   ^ apply-code-action: [C] Extract Variable (this occurrence only)
#   ^ apply-code-action: [D] Extract Variable (all 2 occurrences)
  end
end
