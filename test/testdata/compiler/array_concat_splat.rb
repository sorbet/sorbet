# frozen_string_literal: true
# typed: true
# compiled: true

a = [1, 2, 3]
b = [4, 5, *a, 6]
p b

c = 3
d = [*c, 4, 5, 6]
p d
