# @typed
{}

{a: 'a'}

{**x}
x

{a: 'a', **x} # desugars into
{a: 'a'}.merge(x)

{**x, a: 'a'} # desugars into
x.merge(a: 'a')

{a: 'a', **x, **y, b: 'b', c: 'c', **y, d: 'd'} # desugars into
{a: 'a'}.merge(x).merge(y).merge(b: 'b', c: 'c').merge(y).merge(d: 'd')
