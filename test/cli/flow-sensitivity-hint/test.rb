# typed: true
extend T::Sig

sig {params(x: T::Array[Integer]).void}
def good_example0(x)
  unless x.first.nil?
    x.first.even?
  end
end


sig {params(x: T::Array[Integer]).void}
def good_example1(x)
  if x.first.is_a?(Integer)
    x.first.even?
  end
end

sig {params(x: T::Array[Integer]).void}
def good_example2(x)
  x[0].even? if !x[0].nil?
end


sig {params(x: T::Array[Integer]).void}
def good_example3(x)
  return false if x.first.nil?

  x.first.even?
end

sig {params(x: T::Array[Integer]).void}
def good_example4(x)
  first_is_even = !x.first.nil? && x.first.even?
  first_is_even
end

sig {returns(T.nilable(Integer))}
def looks_like_var_but_is_method
  0
end

sig {params(x: T::Array[Integer]).void}
def good_example5(x)
  if !looks_like_var_but_is_method.nil?
    looks_like_var_but_is_method.even?
  end
end

sig {returns(Class)}
def returns_class
  Integer
end

sig {void}
def good_example6
  if returns_class < Integer
    returns_class.sqrt(4)
  end
end

sig {params(x: T::Array[Integer]).void}
def good_example7(x)
  if x.first == 0
    x.first.even?
  end
end

def divider
  # Cute hack to get something like a divider in the error output
  T.reveal_type("----- Below tests should not have special messages ---------------------------------------------")
end

sig {params(x: T::Array[Integer]).void}
def bad_example1(x)
  unless x.first.nil? # Should NOT have special message
    y = T.let(nil, T.nilable(Integer))
    y.even?
  end
end

class A < T::Struct
  prop :first, T.nilable(String)
end

sig {params(x: T::Array[Integer]).void}
def bad_example2(x)
  unless x.first.nil? # Should NOT have special message
    x = A.new
    x.first.even?
  end
end

class B < T::Struct
  prop :first, T.nilable(Integer)
end

sig {params(x: T::Array[Integer]).void}
def bad_example3(x)
  unless x.first.nil? # Should NOT have special message, but does (bug, annoying to fix)
    x = B.new
    x.first.even?
  end
end

class C < T::Struct
  prop :first, T.untyped
end

sig {params(x: C).void}
def bad_example4(x)
  unless x.first.nil? # Should NOT have special message
    x = T::Array[Integer].new
    x.first.even?
  end
end

sig {params(x: T::Array[Integer]).void}
def bad_example5(x)
  # There is a note about why we don't support this case at the moment in the implementation
  case x.first
  when Integer
    x.first.even?
  end
end

sig {params(x: T::Array[Integer]).void}
def bad_example6(x)
  if 0 == x.first # Should have special message, but doesn't (bug, annoying to fix)
    x.first.even?
  end
end

sig {params(x: T::Array[Integer]).void}
def bad_example7(x)
  if Integer === x.first # Should have special message, but doesn't (bug, annoying to fix)
    x.first.even?
  end
end

sig {params(x: T::Array[T.class_of(Integer)]).void}
def bad_example7(x)
  if x.first === 0 # Should not have special message, but does (bug, annoying to fix)
    x.first.even?
  end
end

