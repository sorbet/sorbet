# frozen_string_literal: true
# typed: strict
# compiled: true
h = {}.compare_by_identity
strs = ('a'..'z').to_a
strs.each { |s| h[s] = s }
200_000.times { strs.each { |s| h[s] } }
