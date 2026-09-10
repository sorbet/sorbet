# typed: true

module Prelude::Second
  class B
    def using_non_imported_constant
      Prelude::First::A.new # error: `Prelude::First` is not imported
    end
  end
end
