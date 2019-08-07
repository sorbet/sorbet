# typed: strict

A = 2 + 1 # works in simple cases

B1 = [1].sum # works with generics
B2 = [1].map{|x| x.to_s}

C = B1 + 1 # does only a single iteration at a time

E = F # doesn't do forward references
F = 0

G = ["foo", 1, :symbol] # widens proxy types
