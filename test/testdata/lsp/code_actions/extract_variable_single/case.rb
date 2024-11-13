# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-variable: true

def extract_cond
  case T.unsafe(1)
#      ^^^^^^^^^^^ apply-code-action: [A] Extract Variable (this occurrence only)
  when Integer
    2
  else
    ""
  end
end

def extract_from_when_pattern
  case T.unsafe(1)
  when Integer
#      ^^^^^^^ apply-code-action: [B] Extract Variable (this occurrence only)
    2
  else
    ""
  end
end

def extract_from_when_body
  case T.unsafe(1)
  when Integer
    2
#   ^ apply-code-action: [C] Extract Variable (this occurrence only)
  else
    ""
  end
end

def extract_from_else_body
  case T.unsafe(1)
  when Integer
    2
  else
    ""
#   ^^ apply-code-action: [D] Extract Variable (this occurrence only)
  end
end