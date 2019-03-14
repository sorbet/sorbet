# typed: true
def p(*args); end;
def tap; end;
p p{p(p);p p}, tap do end
