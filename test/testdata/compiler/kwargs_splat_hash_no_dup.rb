# frozen_string_literal: true
# typed: true
# compiled: true

# We look for sorbet_magic_toHashDup instead of rb_to_hash_type followed by rb_hash_dup,
# because in the INITIAL stage, sorbet_magic_toHashDup has not been inlined yet.

def foo(**kwargs)
  puts(**kwargs)
end


def main
  args = {a: 1, b: 2}
  foo(**args)
end
