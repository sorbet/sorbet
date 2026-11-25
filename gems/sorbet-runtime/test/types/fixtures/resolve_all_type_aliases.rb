# typed: true
# frozen_string_literal: true
require_relative '../../../lib/sorbet-runtime'

# This fixture runs in a subprocess so that ObjectSpace iteration
# is isolated from the main test process.

A1 = T.type_alias { Integer }.checked(:always)
A2 = T.type_alias { String }.checked(:never)

gc = GC
a1 = A1
a2 = A2

before = gc.stat(:total_allocated_objects)
a1.effective_aliased_type
allocs_first = gc.stat(:total_allocated_objects) - before

before = gc.stat(:total_allocated_objects)
a1.effective_aliased_type
allocs_second = gc.stat(:total_allocated_objects) - before

T::Utils.run_all_type_alias_blocks

before = gc.stat(:total_allocated_objects)
a2.effective_aliased_type
allocs_after = gc.stat(:total_allocated_objects) - before

if allocs_first > 0 && allocs_second == 0 && allocs_after == 0
  puts "PASS"
else
  puts "FAIL: first=#{allocs_first} (expected >0), second=#{allocs_second} (expected 0), after_run_all=#{allocs_after} (expected 0)"
  exit 1
end
