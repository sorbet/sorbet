# typed: true

module Consumer
  module Known
  end
end

class ::Outside # error: Defining a root-scoped constant requires this package to be marked `prelude!`
  extend Consumer::Missing
  #      ^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `Missing`
  #      ^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `Missing`
  #      ^^^^^^^^^^^^^^^^^ error: `extend` may only be used on constants in the package that owns them
  extend Consumer::Known # error: `extend` may only be used on constants in the package that owns them
end
