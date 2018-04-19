# typed: strict
def untyped; end

def test_dynamic
  x = untyped
  x && x.y
end
