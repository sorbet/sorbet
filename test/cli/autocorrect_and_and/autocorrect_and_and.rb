# typed: true

class A
  extend T::Sig

  sig {returns(T.nilable(Integer))}
  def foo; nil; end

  def bar
    a = A.new

    if a.foo && a.foo.even?
      puts a.foo
    end

    if a.foo &&
        a.foo.even?
      puts a.foo
    end

    if foo &&
        foo.even?
      puts a.foo
    end
  end
end
