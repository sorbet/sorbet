# typed: true

module Consumer
end

class ::Outside # error: Defining a root-scoped constant requires this package to be marked `prelude!`
  include Consumer::Missing
#         ^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `Missing`

end
