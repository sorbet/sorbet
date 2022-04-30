# typed: true

class Box
  extend T::Sig
  extend T::Generic

  Elem = type_member

  sig {params(x: Elem).void}
  def foo(x)
    T.reveal_type(x) # error: `Box::Elem`
    x.length  # error: Call to method `length` on unbounded type member `Box::Elem`
  end
end

class StringBox < Box
  extend T::Sig
  Elem = type_member {{fixed: String}}

  sig {params(x: Elem).void}
  def bar(x)
    T.reveal_type(x) # error: `String`
    T.reveal_type(x.length) # error: `Integer`
  end
end

Box[Integer].new.foo(0) # would not have `length` method
StringBox.new.foo('')
