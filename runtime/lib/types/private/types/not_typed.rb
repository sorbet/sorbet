# frozen_string_literal: true
# typed: true

# A placeholder for when an untyped thing must provide a type.
# Raises an exception if it is ever used for validation.
class T::Private::Types::NotTyped < T::Types::Base
  ERROR_MESSAGE = "Validation is being done on a `NotTyped`. Please report to #dev-productivity."

  # @override Base
  def name
    "<NOT-TYPED>"
  end

  # @override Base
  def valid?(obj)
    raise ERROR_MESSAGE
  end

  # @override Base
  private def subtype_of_single?(other)
    raise ERROR_MESSAGE
  end
end
