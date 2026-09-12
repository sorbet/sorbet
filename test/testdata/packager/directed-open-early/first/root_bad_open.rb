# typed: true

module ::Second
     # ^^^^^^^^ error: Defining a root-scoped constant requires this package to be marked `prelude!`
  class Foo
  end
end
