# typed: strict

def hazTwoArgs(a, b); 1; end;

def baz
  if foo
    []
  else 
    "[]"
  end
end

Sorbet.sig.void
def give_me_void; end

def bla; give_me_void; end

def bbq
  if foo
    give_me_void
  else
    nil
  end
end

def idk(a); a / a + a * a; end

def give_me_literal; 1; end;

def give_me_literal_nested; [[1]]; end;

