# frozen_string_literal: true
# typed: true

# Use as a mixin with extend (`extend T::Packages`).
module T
  module Packages
    private def package_private(*args)
      args
    end

    private def package_private_class_method(*args)
      args
    end
  end
end
