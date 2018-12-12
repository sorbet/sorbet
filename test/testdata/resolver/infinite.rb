# typed: true
extend T::Sig

S = S # error: Class alias aliases to itself
sig {params(s: S).void}; # error: Constant `S` is not a class or type alias
def f(s); end

sig {params(s: U60).void} # current resolution limit is 30, but because we're iterative we can resolve past it.
def f_long(s); end

U0 = Integer
U1 = U0
U2 = U1
U3 = U2
U4 = U3
U5 = U4
U6 = U5
U7 = U6
U8 = U7
U9 = U8
U10 = U9
U11 = U10
U12 = U11
U13 = U12
U14 = U13
U15 = U14
U16 = U15
U17 = U16
U18 = U17
U19 = U18
U20 = U19
U21 = U20
U22 = U21
U23 = U22
U24 = U23
U25 = U24
U26 = U25
U27 = U26
U28 = U27
U29 = U28
U30 = U29
U31 = U30
U32 = U31
U33 = U32
U34 = U33
U35 = U34
U36 = U35
U37 = U36
U38 = U37
U39 = U38
U40 = U39
U41 = U40
U42 = U41
U43 = U42
U44 = U43
U45 = U44
U46 = U45
U47 = U46
U48 = U47
U49 = U48
U50 = U49
U51 = U50
U52 = U51
U53 = U52
U54 = U53
U55 = U54
U56 = U55
U57 = U56
U58 = U57
U59 = U58
U60 = U59
