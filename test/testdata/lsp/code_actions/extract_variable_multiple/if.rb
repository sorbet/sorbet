# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-variable: true

def different_if_else(x)
  if T.unsafe(nil)
    x + 1
#   ^^^^^ apply-code-action: [A] Extract Variable (this occurrence only)
#   ^^^^^ apply-code-action: [B] Extract Variable (all 2 occurrences)
    x + 1
  else
    x + 2
    x + 2
  end
end

def same_if_else(x)
  if T.unsafe(nil)
    x + 1
#   ^^^^^ apply-code-action: [C] Extract Variable (this occurrence only)
#   ^^^^^ apply-code-action: [D] Extract Variable (all 4 occurrences)
    x + 1
  else
    x + 1
    x + 1
  end
end

def same_cond_if(x)
  if x + 1
    x + 1
#   ^^^^^ apply-code-action: [E] Extract Variable (this occurrence only)
#   ^^^^^ apply-code-action: [F] Extract Variable (all 3 occurrences)
    x + 1
  else
    x + 2
    x + 2
  end
end

def both_match_in_expr(x)
  if x + x
#    ^ apply-code-action: [G] Extract Variable (this occurrence only)
#    ^ apply-code-action: [H] Extract Variable (all 2 occurrences)
    1 + 1
#   ^ apply-code-action: [I] Extract Variable (this occurrence only)
#   ^ apply-code-action: [J] Extract Variable (all 2 occurrences)
  else
    2 + 2
#   ^ apply-code-action: [K] Extract Variable (this occurrence only)
#   ^ apply-code-action: [L] Extract Variable (all 2 occurrences)
  end
end

def if_inside_if(x)
  if T.unsafe(3)
    if x + x
  #    ^ apply-code-action: [M] Extract Variable (this occurrence only)
  #    ^ apply-code-action: [N] Extract Variable (all 2 occurrences)
      1
    else
      2
    end
  end
end
