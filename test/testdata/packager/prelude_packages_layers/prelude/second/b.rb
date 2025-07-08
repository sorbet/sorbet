# typed: true

module Prelude::Second
  class B
    def using_non_imported_constant
      Prelude::First::A.new
    end
  end
end
