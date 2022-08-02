# typed: true
extend T::Sig

def do_something; end

sig {returns(Integer)}
def returns_integer
  0
end

x = T.let(0, Integer)
y = T.let(nil, NilClass)

unless x
  puts x
end

if y
  puts y
end

sig {params(x: Integer).void}
def test1_variable_or(x)
  x || do_something
end

sig {params(x: NilClass).void}
def test2_variable_and(x)
  x && do_something
end

sig {void}
def test3_method_or_method

  returns_integer || do_something
end

sig {params(x: T.any(Integer, String)).void}
def test4_exhaustive_case(x)
  # This test case points the blame back to the `when String` comparison, which
  # is not "wrong" per se. Maybe the user was expecting to see the error blame
  # to `case x`, or perhaps say something like "the case was exhaustive"
  case x
  when Integer then puts 'int'
  when String then puts 'str'
  else
    do_something
  end
end

sig {void}
def test5_if_else
  if returns_integer
    do_something
  else
    do_something
  end
end


sig {void}
def test6_possibly_uninitialized
  if T.unsafe(nil)
    x = 1
  end

  if x
    do_something
  elsif x
    do_something
  end
end

# Can't write this test at the top level because at top level `self` is
# `Object` which is possibly nil and thus possibly falsy 🙃
class A
  extend T::Sig
  sig {void}
  def test7_self
    unless self
      do_something
    end
  end

  sig {void}
  def test8_self_this
    this = self
    unless this
      do_something
    end
  end
end

sig {void}
def test8_cyclic_cfg
  if false
    # This begin/while is the way Ruby encodes do/while.
    begin
      puts 3
    end while T.unsafe(nil)
  end

  puts 5
end
