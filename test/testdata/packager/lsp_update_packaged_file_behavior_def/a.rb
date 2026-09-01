# frozen_string_literal: true
# typed: strict

module Package
  module Namespace
    class A
      extend T::Sig

      sig { void }
      def self.a
      end
    end
  end
end

