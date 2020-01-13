# typed: true
# compiled: true
class A; 
  def foo;
  end
end;
def doubleCast(a)
  T.cast(a, A).foo
  T.cast(a, A).foo
end
