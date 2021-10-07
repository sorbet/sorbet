# typed: strict

module Opus::Util
  class UtilClass; end

  module Nesting # export_for_test
    extend T::Sig
    sig {void}
    def self.nesting_method; end

    class OnlyForSelf; end

    class Public # export
      extend T::Sig
      sig {void}
      def self.public_method; end
    end
  end
end
