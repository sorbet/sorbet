# typed: true
extend T::Sig

module M
  def bar; end
  def foo; end
  def qux; end
end

module Outer
  module Inner; end
end

sig {params(x: M).void}
def test_completion_after_method(x)
  puts 'before'
  # this is special, because we should probably treat this as `x. ; puts ...`
  x.
  # ^ completion: bar, foo, qux, ...
end # error: unexpected token "end"

sig {params(x: M).void}
def test_completion_between_keywords(x)
  begin; end
  x.
  # ^ completion: bar, foo, qux, ...
  begin; end # error: unexpected token "begin"
end

# The weird thing is that this is actually a valid Ruby program.
sig {params(x: M).void}
def test_completion_before_method(x)
  x.
  # ^ completion: bar, foo, qux, ...
  puts 'after' # error: Method `puts` does not exist on `M`
end

class TestClass1
  extend T::Sig
  def before; end

  sig {params(x: M).void}
  def test_method_in_class(x)
    x.
    # ^ completion: bar, foo, qux, ...
  end # error: unexpected token "end"

  def after; end
end

sig {params(x: M).void}
def test_lonely_dot(x)
  x
    .
  #  ^ completion: bar, foo, qux, ...
end # error: unexpected token "end"

sig {params(x: M).void}
def test_inside_parens(x)
  puts(x.) # error: unexpected token ")"
  #      ^ completion: bar, foo, qux, ...
end

sig {params(x: M).void}
def test_first_arg_no_parens(x)
  puts x.
  #      ^ completion: bar, foo, qux, ...
end # error: unexpected token "end"

# The weird thing is that this is actually a valid Ruby program.
sig {params(x: M).void}
def test_before_var_assign(x)
  x.
  # ^ completion: bar, foo, qux, ...
  y = nil # error: Setter method `y=` does not exist on `M`
end

# This is also technically a valid Ruby program, but Sorbet doesn't support it
sig {params(x: M).void}
def test_before_constant_lit(x)
  x.
  # ^ completion: bar, foo, qux, ...
  Outer # error: does not exist

  x. # error: Dynamic constant reference
  # ^ completion: bar, foo, qux, ...
  Outer::Inner # error: Method `Outer` does not exist on `M`
end

sig {params(x: M).void}
def test_csend(x)
  x&.
  #  ^ completion: bar, foo, qux, ...
end # error: unexpected token "end"

