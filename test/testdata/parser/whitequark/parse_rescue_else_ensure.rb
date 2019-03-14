# typed: true
def meth; end;
def baz; end;
def foo; end;
def bar; end;

begin; meth; rescue; baz; else foo; ensure; bar end
