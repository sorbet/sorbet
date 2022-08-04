# typed: true

expr = []

case expr
in [] => a
  T.reveal_type(a) # error: Revealed type: `T.untyped`
in [b]
  T.reveal_type(b) # error: Revealed type: `T.untyped`
in [c, [d]]
  T.reveal_type(c) # error: Revealed type: `T.untyped`
  T.reveal_type(d) # error: Revealed type: `T.untyped`
in e, [f], g => h
  T.reveal_type(e) # error: Revealed type: `T.untyped`
  T.reveal_type(f) # error: Revealed type: `T.untyped`
  T.reveal_type(g) # error: Revealed type: `T.untyped`
  T.reveal_type(h) # error: Revealed type: `T.untyped`
in [i,]
  T.reveal_type(i) # error: Revealed type: `T.untyped`
in [k, *, l]
  T.reveal_type(k) # error: Revealed type: `T.untyped`
  T.reveal_type(l) # error: Revealed type: `T.untyped`
in [m, *n]
  T.reveal_type(m) # error: Revealed type: `T.untyped`
  T.reveal_type(n) # error: Revealed type: `T.untyped`
else
end
