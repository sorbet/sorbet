# typed: true
def foo
    a = 1..2
    b = Range.new(1, 2)
    c = 1...2
    d = Range.new(1, 2, true)
    e = (1..42).first
    f = ('a'..'z').last

    T.reveal_type(a) # error: Revealed type: `T::Range[Integer]`
    T.reveal_type(b) # error: Revealed type: `T::Range[T.untyped]`
    T.reveal_type(c) # error: Revealed type: `T::Range[Integer]`
    T.reveal_type(d) # error: Revealed type: `T::Range[T.untyped]`
    T.reveal_type(e) # error: Revealed type: `Integer`
    T.reveal_type(f) # error: Revealed type: `String`

    # testing for endless ranges
    g = (1..)
    h = Range.new(1, nil)
    T.reveal_type(g) # error: Revealed type: `T::Range[Integer]`
    T.reveal_type(h) # error: Revealed type: `T::Range[T.untyped]`

    i = (1.."a")
    T.reveal_type(i) # error: Revealed type: `T::Range[T.untyped]`
end
