# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-variable: true

class A
  attr_accessor :a
end

def rhs_of_asgn(x)
  A.new.a ||= 1
#             ^ apply-code-action: [A] Extract Variable (this occurrence only)
#             ^ apply-code-action: [B] Extract Variable (all 6 occurrences)
  A.new.a &&= 1
  A.new.a *= 1
  x ||= 1
  x &&= 1
  x *= 1
end

def part_of_lhs_of_asgn
  A.new.a ||= 1
# ^^^^^ apply-code-action: [C] Extract Variable (this occurrence only)
# ^^^^^ apply-code-action: [D] Extract Variable (all 4 occurrences)
  A.new.a &&= 1
  A.new.a *= 1
  puts(A.new)
end
