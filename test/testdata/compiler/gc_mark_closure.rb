# frozen_string_literal: true
# typed: strict
# compiled: true
h = {}

1.upto(1){|i|
  GC.start(full_mark: false, immediate_sweep: false)
  T.unsafe(GC).verify_compaction_references(double_heap: true, toward: :empty)
  h[1] = 1 
}
