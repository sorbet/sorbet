# frozen_string_literal: true
# typed: true

# Holds a string. Useful for showing type aliases in error messages
class T::Private::Types::StringHolder < T::Types::Base
  attr_reader :string

  def initialize(string)
    @string = string
  end

  # @override Base
  def name
    string
  end

  # @override Base
  def valid?(obj)
    false
  end

  # @override Base
  private def subtype_of_single?(other)
    false
  end
end
