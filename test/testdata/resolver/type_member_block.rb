# typed: true

module IBox1
  extend T::Generic
  X = type_member(:out) {{upper: T.any(Integer, String)}}
end

module IBox2
  extend T::Generic
  X = type_member(:out) {{'upper' => T.any(Integer, String)}}
  #                       ^^^^^^^ error: must have symbol keys
end

module IBox3
  extend T::Generic
  X = type_member(:out, fixed: Integer) {{upper: T.any(Integer, String)}}
  #   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `type_member` must not have both keyword args and a block
  #   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Type member is defined with bounds and `fixed`
end

module IBox4
  extend T::Generic
  X = type_member(:out) {nil}
  #                      ^^^ error: Block given to `type_member` must contain a single `Hash` literal
end

module IBox5
  extend T::Generic
  X = type_member(:out) {x = 1; p(x); {upper: Integer}}
  #                      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Block given to `type_member` must contain a single `Hash` literal
end

module IBox6
  extend T::Generic
  X = type_member(:out) {{unknown: Integer}}
  #                       ^^^^^^^ error: Unknown key `unknown` provided in block to `type_member`
end
