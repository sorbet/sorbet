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

    # Wildly, this is valid method call syntax.
    # Our current implementation will not trigger (not even the error).
    if a . foo &&
        a.foo.even?
      puts a.foo
    end

    # Our autocorrect is very simplicit right now.
    # It will not fire when the `.` and method name are on separate lines.
    if a.foo && a.foo.
        even?
      puts a.foo
    end

  end
end

extend T::Sig

class Auth
  extend T::Sig
  sig { returns(T::Boolean) }
  def livemode
    true
  end
end

sig { returns(T.nilable(Auth))}
def auth
end

(auth && auth.livemode)
