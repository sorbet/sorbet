# typed: true

class HasPrivate
  private def my_private; end
end

class A
  extend T::Sig

  sig {returns(T.nilable(Integer))}
  def foo; nil; end

  sig {returns(T.nilable(HasPrivate))}
  def get_private; end

  def bar
    a = A.new

    if a.foo && a.foo.even?
      #               ^^^^^ error: Call to method `even?` after `&&` assumes result type doesn't change
      puts a.foo
    end

    if foo && foo.even?
      #           ^^^^^ error: Call to method `even?` after `&&` assumes result type doesn't change
      puts a.foo
    end

    if a.get_private && a.get_private.my_private
      #                               ^^^^^^^^^^ error: Non-private call to private method `my_private`
      #                               ^^^^^^^^^^ error: Call to method `my_private` after `&&` assumes result type doesn't change
      puts a.foo
    end
  end
end
