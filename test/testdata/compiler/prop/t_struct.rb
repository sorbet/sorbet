# frozen_string_literal: true
# typed: true
# compiled: true

class A < T::Struct
  prop :int, Integer
  prop :str, String
  prop :optint, Integer, default: 5
  prop :optarray, T::Array[Integer], default: [6]
  prop :optstr, String, default: "shared string"
end

# Verify default arguments don't share structure.
x = A.new(int: 5, str: "foo")
y = A.new(int: 6, str: "bar")
p x.optarray.object_id != y.optarray.object_id
# ...but it's OK if the strings do, given that we're using frozen_string_literal.
p x.optstr.object_id == y.optstr.object_id

def verify_runtime_error(&blk)
  begin
    yield
  rescue => e
    # We're not particularly concerned about the error message.
    p e.class
  else
    p "not supposed to get here"
  end
end

# Verify runtime type-checking is done on required args.
verify_runtime_error {A.new(int: T.unsafe(:not_an_integer), str: "foo")}
verify_runtime_error {A.new(int: 4, str: T.unsafe(:not_a_string))}

# Verify runtime type-checking is done on optional args.
verify_runtime_error{A.new(int: 5, str: "foo", optint: T.unsafe("something"))}
verify_runtime_error{A.new(int: 5, str: "foo", optint: 18, optarray: T.unsafe(34))}
verify_runtime_error{A.new(int: 5, str: "foo", optstr: T.unsafe(29))}

# Verify extra keyword arguments are not accepted.
verify_runtime_error{A.send(:new, int: 6, str: "foo", notarg: 19)}
verify_runtime_error{A.send(:new, 13, int: 7, str: "bar")}

class HasEnumMember < T::Struct
  prop :lame_enum, String, enum: ["value1", "value2"]
end

verify_runtime_error {HasEnumMember.new(lame_enum: "bad_value")}
