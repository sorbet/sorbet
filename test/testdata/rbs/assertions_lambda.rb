# typed: strict
# enable-experimental-rbs-comments: true

lam1 = -> {
  42 #: as String
}
T.reveal_type(lam1) # error: Revealed type: `T.proc.returns(String)`

lam2 = -> {
  x = 1
  x #: as String
}
T.reveal_type(lam2) # error: Revealed type: `T.proc.returns(String)`

lam3 = lambda do
  42 #: as String
end
T.reveal_type(lam3) # error: Revealed type: `T.proc.returns(String)`
