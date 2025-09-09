# typed: true

module Prelude::Second
  class B
    def using_explicitly_imported_constant
      Prelude::First::A.new
    end
  end
end
