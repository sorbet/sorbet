# typed: false
def foo
    a += a
    a[b][c][d, e] += 3
    a.b.c[d.e] += 4
    a.b.c[d.e] = a.b.c[d.e] + 4
end
