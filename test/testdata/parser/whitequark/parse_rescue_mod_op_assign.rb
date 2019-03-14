# typed: true
def meth; end;
def bar; end
foo += meth rescue bar # error: does not exist on `NilClass`
