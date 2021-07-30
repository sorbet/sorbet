# frozen_string_literal: true
# typed: strict
# compiled: true
h = {}
strs = [*1..10000].map! {|i| i.fdiv(10)}
strs.each { |s| h[s] = s }
50.times { strs.each { |s| h[s] } }
