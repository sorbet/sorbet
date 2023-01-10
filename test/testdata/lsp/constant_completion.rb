# typed: true

class A
  module Namespace; end
  Namespac # error: Unable to resolve constant `Namespac`
  #       ^ completion: Namespace
end

class B
  module Namespace; end
  X = Namespac # error: Unable to resolve constant `Namespac`
  #           ^ completion: Namespace
end
