# typed: true
extend T::Sig

module M
  def foo; end
  def bar; end
  def qux; end
end

sig {params(x: M).void}
def test_completion_before_method(x)
  puts 'before'
  # this is special, because we should probably treat this as `x. ; puts ...`
  x.
  # ^ completion: bar, foo, qux, ...
end # error: unexpected token "end"

