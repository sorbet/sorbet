# frozen_string_literal: true
# typed: true
# compiled: true

a = [1, 2, 3]
b = [4, 5, *a, 6]
p a
p b

c = 3
d = [*c, 4, 5, 6]
p c
p d

e = [8, 9]
f = [*e, 7, 6, 5]
p e
p f

g = [10, 11]
h = [1, 2, 3, 4, *g]
p g
p h
