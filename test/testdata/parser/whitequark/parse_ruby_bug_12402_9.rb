# typed: true
def foo; end;
def bar; end;
foo[0] += raise bar rescue nil
