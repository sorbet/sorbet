# frozen_string_literal: true
# typed: strong
# compiled: true
arr = [*0...100000]
10_000.times {arr.sample 10_000}
