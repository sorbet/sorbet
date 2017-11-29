{}

{a: 'a'}

{**x}
x

{a: 'a', **x} # desugars into
{a: 'a'}.merge(x.to_hash)

{**x, a: 'a'} # desugars into
(x.to_hash).merge(a: 'a')

{a: 'a', **x, **y, b: 'b', c: 'c', **y, d: 'd'} # desugars into
{a: 'a'}.merge(x.to_hash).merge(y.to_hash).merge(b: 'b', c: 'c').merge(y.to_hash).merge(d: 'd')
