# typed: true

# https://github.com/sorbet/sorbet/issues/1731

class Box
  extend T::Sig
  extend T::Generic

  Elem = type_member

  sig {params(x: Elem).void}
  def foo(x)
    # correct
    T.reveal_type(x) # error: `Box::Elem`

    # incorrect--no error for method not existing
    len = x.length
    T.reveal_type(len) # error: `T.untyped`
  end
end

class StringBox < Box
  extend T::Sig
  Elem = type_member {{fixed: String}}

  sig {params(x: Elem).void}
  def bar(x)
    # correct
    T.reveal_type(x) # error: `String`

    # probably correct to say that method exists?
    len = x.length
    T.reveal_type(len) # error: `Integer`
  end
end

StringBox.new.foo('')
