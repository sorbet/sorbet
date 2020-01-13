# typed: strict

class BoxA
#     ^ type-def: BoxA
#     ^ type-def: AB
  extend T::Generic
  Elem = type_member
end

class BoxB
#     ^ type-def: AB
  extend T::Generic
  Elem = type_member
end

class TestClass
  box_a = BoxA[Integer].new
  puts box_a
  #    ^ type: BoxA

  ab = T.let(BoxA[Integer].new, T.any(BoxA[Integer], BoxB[String]))
  puts ab
  #    ^ type: AB

  bare_box_a = BoxA.new
  puts T.reveal_type(bare_box_a) # error: Revealed type: `BoxA[T.untyped]`
  #                  ^ type: BoxA

  # TODO for https://github.com/sorbet/sorbet/issues/1777
  tup = [ 0 ]
  T.reveal_type(tup) # error: [Integer(0)] (1-tuple)
  #             ^ hover [Integer(0)] (1-tuple)
  T.reveal_type([ 0 ]) # error: [Integer(0)] (1-tuple)
  #                 ^ TODO: hover: [Integer(0)] (1-tuple)
end
