# typed: true
extend T::Helpers

sig {params(a: T.nilable(Integer)).void}
def foo1(a)
  if !(a == nil)
     T.assert_type!(a, Integer)
  end
end


sig {params(a: T.any(Integer, TrueClass)).void}
def foo2(a)
  if !(a == true)
     T.assert_type!(a, Integer)
  end
end


sig {params(a: T.any(Integer, FalseClass)).void}
def foo3(a)
  if !(a == false)
     T.assert_type!(a, Integer)
  end
end

sig {params(a: T.nilable(Integer)).void}
def nfoo1(a)
  if a != nil
     T.assert_type!(a, Integer)
  end
end


sig {params(a: T.any(Integer, TrueClass)).void}
def nfoo2(a)
  if a != true
     T.assert_type!(a, Integer)
  end
end


sig {params(a: T.any(Integer, FalseClass)).void}
def nfoo3(a)
  if a != false
     T.assert_type!(a, Integer)
  end
end
