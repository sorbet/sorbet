class HasPrivate
  private def my_private; end
end

class A
  extend T::Sig

  sig {abstract.params(blk: T.proc.returns(Out)).void}
  def foo
    T.reveal_type(@y1)
    T.reveal_type(@y2)

    @z = T.let(nil, T.nilable(T.type_parameter(:U)))
  end

  sig {returns(T.nilable(HasPrivate))}
  def ;end; end

  def bar
    a = A.new

    if a.foo && a.foo.even?
      puts a.foo
    end

    if foo && foo.even?
      puts a.foo
    end

    if a.get_private && a.get_private.my_private
      puts a.foo
    end
  end
end
