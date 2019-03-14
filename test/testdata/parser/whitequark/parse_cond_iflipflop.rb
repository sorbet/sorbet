# typed: true
def foo; end;
def bar; end;
if foo..bar; end # error: Unsupported node type `IFlipflop`
