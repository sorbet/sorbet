# frozen_string_literal: true
# typed: strong
# compiled: true
h = {}

1.upto(1){|i|
  GC.start(full_mark: false, immediate_sweep: false)
  h[1] = 1 
}
