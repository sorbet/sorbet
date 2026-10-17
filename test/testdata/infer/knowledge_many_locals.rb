# typed: strict
extend T::Sig

# Every local a branch reads carries a type test on every local in scope, so with this many locals those lists are
# long enough to be indexed.

sig {params(x: Integer, y: T.nilable(String), cond: T::Boolean).void}
def many_locals(x, y, cond)
  l0 = x > 0; l1 = x > 1; l2 = x > 2; l3 = x > 3; l4 = x > 4; l5 = x > 5; l6 = x > 6; l7 = x > 7; l8 = x > 8; l9 = x > 9
  l10 = x > 10; l11 = x > 11; l12 = x > 12; l13 = x > 13; l14 = x > 14; l15 = x > 15; l16 = x > 16; l17 = x > 17; l18 = x > 18; l19 = x > 19
  l20 = x > 20; l21 = x > 21; l22 = x > 22; l23 = x > 23; l24 = x > 24; l25 = x > 25; l26 = x > 26; l27 = x > 27; l28 = x > 28; l29 = x > 29
  l30 = x > 30; l31 = x > 31; l32 = x > 32; l33 = x > 33; l34 = x > 34; l35 = x > 35; l36 = x > 36; l37 = x > 37; l38 = x > 38; l39 = x > 39
  l40 = x > 40; l41 = x > 41; l42 = x > 42; l43 = x > 43; l44 = x > 44; l45 = x > 45; l46 = x > 46; l47 = x > 47; l48 = x > 48; l49 = x > 49
  l50 = x > 50; l51 = x > 51; l52 = x > 52; l53 = x > 53; l54 = x > 54; l55 = x > 55; l56 = x > 56; l57 = x > 57; l58 = x > 58; l59 = x > 59
  l60 = x > 60; l61 = x > 61; l62 = x > 62; l63 = x > 63; l64 = x > 64; l65 = x > 65; l66 = x > 66; l67 = x > 67; l68 = x > 68; l69 = x > 69
  if cond
    v = true
    s = 1
  else
    v = false
    s = nil
  end
  if v
    T.reveal_type(s) # error: Revealed type: `Integer(1)`
  end
  if l3 && l7
    T.reveal_type(l3) # error: Revealed type: `TrueClass`
  end
  t = l5 ? s : 2
  T.reveal_type(t) # error: Revealed type: `T.nilable(Integer)`
  ok = cond ? !y.nil? && l1 : l2 && !y.nil?
  if ok
    T.reveal_type(y) # error: Revealed type: `String`
  end
end
