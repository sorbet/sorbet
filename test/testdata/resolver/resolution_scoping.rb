# typed: strict
class A < B # error: Unable to resolve constant `B`
  module C
  end
  module B
  end
  include B
  include C
end

module D
  include E
  module E
  end
end

