# typed: true

class A
  extend T::Sig

  sig {returns(T.nilable(Integer))}
  def foo; nil; end

  def bar
    a = A.new

    # Remember: we only show completion results that are valid. Asking for
    # completion results for `a.foo.even?` here would not return any results.
    if a.foo && a.foo.to_s.empty?
      #                   ^ completion: to_s
      puts a.foo
    end
  end
end
