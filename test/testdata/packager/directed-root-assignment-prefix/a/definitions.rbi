# typed: true

::A::FromRBI = T.let(T.unsafe(nil), String) # error: Defining a root-scoped constant requires this package to be marked `prelude!`
