# frozen_string_literal: true
# typed: true
# compiled: true

# We look for sorbet_magic_toHashDup instead of rb_to_hash_type followed by rb_hash_dup,
# because in the INITIAL stage, sorbet_magic_toHashDup has not been inlined yet.


# LOWERED-LABEL: define internal i64 @"func_Object#3foo"
# LOWERED: call i64 @rb_to_hash_type
# LOWERED-NOT: call i64 @rb_hash_dup
# LOWERED{LITERAL}: }

def foo(**kwargs)
  puts(**kwargs)
end


# LOWERED-LABEL: define internal i64 @"func_Object#4main"
# LOWERED: call i64 @rb_to_hash_type
# LOWERED-NOT: call i64 @rb_hash_dup
# LOWERED{LITERAL}: }

def main
  args = {a: 1, b: 2}
  foo(**args)
end
