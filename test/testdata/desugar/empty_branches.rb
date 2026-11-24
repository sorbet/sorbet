# typed: true

# Desugar keeps a branch that has no `else` keyword (or the body of a
# modifier) apart from an explicitly empty branch.

def missing_else(c)
  if c; 1; end
end

def empty_else(c)
  if c; 1; else; end
end

def empty_then(c)
  if c; else; 1; end
end

def empty_if(c)
  if c; end
end

def missing_elsif_else(c, d)
  if c; 1; elsif d; 2; end
end

def empty_elsif_else(c, d)
  if c; 1; elsif d; 2; else; end
end

def empty_elsif(c, d)
  if c; 1; elsif d; else 3; end
end

def unless_missing_else(c)
  unless c; 1; end
end

def unless_empty_else(c)
  unless c; 1; else; end
end

def unless_empty_body(c)
  unless c; else; 1; end
end

def empty_unless(c)
  unless c; end
end

def modifier_if(c)
  1 if c
end

def modifier_unless(c)
  1 unless c
end

def ternary(c)
  c ? 1 : 2
end

def case_missing_else(x)
  case x
  when 1 then 2
  end
end

def case_empty_else(x)
  case x
  when 1 then 2
  else
  end
end

def case_empty_when(x)
  case x
  when 1
  else 3
  end
end

def case_in_missing_else(x)
  case x
  in 1 then 2
  end
end

def case_in_empty_else(x)
  case x
  in 1 then 2
  else
  end
end

def case_in_empty_in(x)
  case x
  in 1
  else 3
  end
end

def dead_empty_body
  if nil; end
end
