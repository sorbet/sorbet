# typed: true
extend T::Sig

class A
  extend T::Sig

  # Docs for A#foo
  sig {returns(String)}
  def foo; ''; end
end

class B
  extend T::Sig

  # Docs for B#foo
  sig {returns(Integer)}
  def foo; 0; end
end

sig {params(x: T.any(A, B)).void}
def multiple(x)
  x.foo
  # ^ hover-line: 1 ```ruby
  # ^ hover-line: 2 sig {returns(String)}
  # ^ hover-line: 3 sig {returns(Integer)}
  # ^ hover-line: 5 -> T.any(String, Integer)
  # ^ hover-line: 6 def foo; end
  # ^ hover-line: 7 ```
  # ^ hover-line: 9 ---
  # ^ hover-line: 11 Docs for A#foo
end

sig {params(x: A).void}
def single(x)
  x.foo
  # ^ hover-line: 1 ```ruby
  # ^ hover-line: 2 sig {returns(String)}
  # ^ hover-line: 3 def foo; end
  # ^ hover-line: 4 ```
  # ^ hover-line: 6 ---
  # ^ hover-line: 8 Docs for A#foo
end
