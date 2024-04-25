# frozen_string_literal: true
# typed: true
# compiled: true

def foo(**kwargs)
  puts kwargs
end


# LOWERED-LABEL: call i64 @rb_to_hash_type
# LOWERED: call i64 @rb_hash_dup
# LOWERED{LITERAL}: }

args = {a: 1, b: 2}
foo(**args)
puts args
