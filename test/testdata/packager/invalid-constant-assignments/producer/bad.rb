# typed: true

::FromRuby = 1 # error: Defining a root-scoped constant requires this package to be marked `prelude!`
::FromRuby = 2 # error: Defining a root-scoped constant requires this package to be marked `prelude!`

module Producer
  ::Consumer::Injected = 3 # error: Defining a root-scoped constant requires this package to be marked `prelude!`
end
