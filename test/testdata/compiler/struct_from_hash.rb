# frozen_string_literal: true
# typed: strict
# compiled: true

class MyStruct < T::Struct
  extend T::Sig

  const :foo, Integer
  const :percent, Float
  const :hash_by, Symbol

  sig {override.params(struct: T::Hash[String, T.untyped], strict: T::Boolean).returns(T.attached_class)}
  def self.from_hash(struct, strict = false)
    if struct.key?('hash_by')
      struct = struct.merge({'hash_by' => struct['hash_by'].to_sym})
    end
    super(struct)
  end
end

s = MyStruct.from_hash({'hash_by' => 'foo', 'foo' => 5, 'percent' => 3.0})

p s.hash_by
p s.serialize
