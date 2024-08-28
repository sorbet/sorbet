# typed: false

a, b = 1, 2

# splat node
a, b, *c = 1, 2, 3, 4

# anonymous splat node
a, * = 1, 2, 3, 4

# parentheses
(a, b, c) = 1, 2, 3

# targets to the right of the splat
a, *b, c = 1, 2, 3, 4

# only splat
*a = 1, 2, 3, 4

# no targets left of the splat
*a, b, c = 1, 2, 3, 4
