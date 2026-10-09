# typed: true

::FromRBI = T.let(1, Integer) # error: Defining a root-scoped constant requires this package to be marked `prelude!`
