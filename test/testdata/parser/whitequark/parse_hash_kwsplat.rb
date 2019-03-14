# typed: true
def bar; end;
{ foo: 2, **bar } # error: Method `merge` does not exist on
