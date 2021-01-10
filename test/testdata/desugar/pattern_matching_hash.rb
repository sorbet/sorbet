# typed: true

expr = {}

case expr
in {a: a}
  T.reveal_type(a) # error: Revealed type: `T.untyped`
in {"kb": b}
  T.reveal_type(b) # error: Revealed type: `T.untyped`
in {"kc": c, kd: d}
  T.reveal_type(c) # error: Revealed type: `T.untyped`
  T.reveal_type(d) # error: Revealed type: `T.untyped`
in {"k#{"e"}": e, kf: {kg: g}, kh: h} => i
  T.reveal_type(e) # error: Revealed type: `T.untyped`
  T.reveal_type(g) # error: Revealed type: `T.untyped`
  T.reveal_type(h) # error: Revealed type: `T.untyped`
  T.reveal_type(i) # error: Revealed type: `T.untyped`
in {"kj": j} | {"kh": l} => m
  T.reveal_type(j) # error: Revealed type: `T.untyped`
  T.reveal_type(l) # error: Revealed type: `T.untyped`
  T.reveal_type(m) # error: Revealed type: `T.untyped`
in {"n1":, n2:, "n3":} => n4
  T.reveal_type(n1) # error: Revealed type: `T.untyped`
  T.reveal_type(n2) # error: Revealed type: `T.untyped`
  T.reveal_type(n3) # error: Revealed type: `T.untyped`
  T.reveal_type(n4) # error: Revealed type: `T.untyped`
in **o
  T.reveal_type(o) # error: Revealed type: `T.untyped`
in **nil
end
