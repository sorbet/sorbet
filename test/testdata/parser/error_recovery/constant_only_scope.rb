# typed: false
class A; end
def test_constant_only_scope
  A::
  #^^ parser-error: expected constant name following "::"
end
