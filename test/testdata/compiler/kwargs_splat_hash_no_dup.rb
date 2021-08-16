# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL
# run_filecheck: LOWERED

# We look for sorbet_magic_toHashDup instead of rb_to_hash_type followed by rb_hash_dup,
# because in the INITIAL stage, sorbet_magic_toHashDup has not been inlined yet.

# INITIAL-LABEL: define i64 @"func_Object#foo"
# INITIAL: call i64 @sorbet_magic_toHashDup
# INITIAL{LITERAL}: }

# LOWERED-LABEL: define i64 @"func_Object#foo"
# LOWERED: call i64 @rb_to_hash_type
# LOWERED-NOT: call i64 @rb_hash_dup
# LOWERED{LITERAL}: }

def foo(**kwargs)
  puts(**kwargs)
end

# INITIAL-LABEL: define i64 @"func_Object#main"
# INITIAL: call i64 @sorbet_magic_toHashDup
# INITIAL{LITERAL}: }

# LOWERED-LABEL: define i64 @"func_Object#main"
# LOWERED: call i64 @rb_to_hash_type
# LOWERED-NOT: call i64 @rb_hash_dup
# LOWERED{LITERAL}: }

def main
  args = {a: 1, b: 2}
  foo(**args)
end
