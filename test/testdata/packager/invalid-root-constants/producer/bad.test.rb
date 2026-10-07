# typed: strict

module ::Producer # error: requires this package to be marked `prelude!`
  class FromTest # error: Defining a root-scoped constant requires this package to be marked `prelude!`
  end
end
