# typed: strict

module Producer
  class FinalTarget
    class Nested; end
  end
  Final = FinalTarget

  module ViaTarget
    class Nested; end
    Value = 1
  end
  Via = ViaTarget
end
