# frozen_string_literal: true
# typed: strict
# compiled: true
h = {}
syms = ('a'..'z').map { |s| s.to_sym }
syms.each { |s| h[s] = 1 }
200_000.times { syms.each { |s| h[s] } }
