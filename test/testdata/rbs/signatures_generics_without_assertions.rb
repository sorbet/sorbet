# typed: strict
# enable-experimental-rbs-signatures: true

#: [U]
class G1; end

g1_1 = G1.new
T.reveal_type(g1_1) # error: Revealed type: `G1[T.untyped]`

g1_2 = G1.new #: G1[Integer]
T.reveal_type(g1_2) # error: Revealed type: `G1[T.untyped]`
