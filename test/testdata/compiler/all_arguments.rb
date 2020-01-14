# typed: true
# compiled: true
def take_arguments(a, b=1, *c, d:, e:2, **f, &g)
    puts [a,b,c,d,e,f,g].inspect
end

take_arguments(-1, -2, -3, -4, -5, -6, -7, d: -8)
take_arguments(-1, -2, -3, -4, -5, -6, d: -8)
take_arguments(-1, -2, -3, -4, -5, d: -8)
take_arguments(-1, -2, -3, -4, d: -8)
take_arguments(-1, -2, -3, d: -8)
take_arguments(-1, -2, d: -8)
take_arguments(-1, d: -8)

take_arguments(-1, -2, -3, -4, -5, -6, -7, d: -8, e: -9)
take_arguments(-1, -2, -3, -4, -5, -6, d: -8, e: -9)
take_arguments(-1, -2, -3, -4, -5, d: -8, e:-9)
take_arguments(-1, -2, -3, -4, d: -8, e: -9)
take_arguments(-1, -2, -3, d: -8, e: -9)
take_arguments(-1, -2, d: -8, e: -9)
take_arguments(-1, d: -8, e: -9)

take_arguments(-1, -2, -3, -4, -5, -6, -7, d: -8, e: -9, baz: -10)
take_arguments(-1, -2, -3, -4, -5, -6, d: -8, e: -9, baz: -10)
take_arguments(-1, -2, -3, -4, -5, d: -8, e: -9, baz: -10)
take_arguments(-1, -2, -3, -4, d: -8, e: -9, baz: -10)
take_arguments(-1, -2, -3, d: -8, e: -9, baz: -10)
take_arguments(-1, -2, d: -8, e: -9, baz: -10)
take_arguments(-1, d: -8, e: -9, baz: -10)

