# typed: true

class A
  def foo; end
  #    ^ def: A#foo
  alias_method :bar1, :foo
  #                    ^ usage: A#foo

  def bar2
    foo
  # ^ usage: A#foo
  end
end
