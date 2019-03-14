# typed: true
def meth; end;
def bar; end;
begin; meth; rescue => ex; bar; end
