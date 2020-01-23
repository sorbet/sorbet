# frozen_string_literal: true
# typed: true
# compiled: true
h = T.unsafe({1=>2})

puts [1].map(&h)
