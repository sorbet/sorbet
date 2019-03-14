# typed: true
def foo; end;
def baz; end;
def bar; end;
case foo; when 1, *baz; bar; when *foo; end
