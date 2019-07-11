# typed: true
def foo; end;
def bar; end;
if foo...bar; end # error-with-dupes: Unsupported node type `EFlipflop`
