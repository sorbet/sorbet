# frozen_string_literal: true
# typed: strict
# compiled: true
h = {}.compare_by_identity
nums = (1..26).to_a
nums.each { |n| h[n] = n }
200_000.times { nums.each { |n| h[n] } }
