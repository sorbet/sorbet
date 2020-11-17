# typed: true

T.let(0, 0)
T.let(0.0, 0.0)
T.let('', '')
T.let(:'', :'')
T.let(true, true)
T.let(false, false)
T.let(nil, nil)

T.let(true, T.any(true, false))
