# typed: true

module Consumer
  module Known
  end
end

class ::Outside # error: Defining a root-scoped constant requires this package to be marked `prelude!`
  include Consumer::Missing
#         ^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `Missing`

  include Consumer::Known
end
