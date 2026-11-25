# typed: strict

module Root::A
  class Exported; end
  class Unexported; end
  UNEXPORTED_CONST = 1

  module Namespace
    class Nested; end
  end
end
