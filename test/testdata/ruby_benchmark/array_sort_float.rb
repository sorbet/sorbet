# frozen_string_literal: true
# typed: strong
# compiled: true
arr = Array.new(1000) { rand }
10000.times { arr.sort }
