# typed: true

class Test
  extend T::Sig

  sig do
    params(
      a: Integer,
      b: Integer,
      c: Integer,
      d: Integer,
      e: Integer,
      f: String,
      blk: T.proc.void,
    ).void
  end
  def test1(a, b, c=10, d=(x=20), e:, f: 'foo', &blk)
    T.reveal_type(a) # error: Revealed type: `Integer`
    T.reveal_type(b) # error: Revealed type: `Integer`
    T.reveal_type(c) # error: Revealed type: `Integer`
    T.reveal_type(d) # error: Revealed type: `Integer`
    T.reveal_type(e) # error: Revealed type: `Integer`
    T.reveal_type(f) # error: Revealed type: `String`
    T.reveal_type(blk) # error: Revealed type: `T.proc.void`
    yield

    T.reveal_type(x) # error: Revealed type: `T.nilable(Integer)`
  end

  sig {params(x: Integer, rest: Integer, blk: T.proc.void).void}
  def test2(x=10, *rest, &blk)
    T.reveal_type(x) # error: Revealed type: `Integer`
    T.reveal_type(rest) # error: Revealed type: `T::Array[Integer]`
    T.reveal_type(blk) # error: Revealed type: `T.proc.void`
  end

  sig {params(x: Integer, rest: Integer, blk: T.proc.void).void}
  def test3(x=10, **rest, &blk)
    T.reveal_type(x) # error: Revealed type: `Integer`
    T.reveal_type(rest) # error: Revealed type: `T::Hash[Symbol, Integer]`
    T.reveal_type(blk) # error: Revealed type: `T.proc.void`
  end
end
