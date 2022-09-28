# typed: strict

class C
  extend T::Sig

  @@foo = T.let(3, Integer)

  sig {void}
  def bar
    @@quz # error: Use of undeclared variable `@@quz`
  end
end

class D < C
  T.reveal_type( # error: `Integer`
    @@foo
  )
  T.reveal_type( # error: `T.untyped`
    @@food # error: Use of undeclared variable `@@food`
  )
end
