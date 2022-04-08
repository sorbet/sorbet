# typed: true
class C
  extend T::Sig

  sig { returns([Integer]) }
  def foo
    [1]
  end

  sig { returns([Integer, String]) }
  def bar
    [2, "bar"]
  end
end

T.reveal_type(C.new.foo) # error: Revealed type: `[Integer] (1-tuple)`
T.reveal_type(C.new.bar) # error: Revealed type: `[Integer, String] (2-tuple)`
