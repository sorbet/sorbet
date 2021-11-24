# typed: strict

module PaymentRecord
  extend T::Helpers
  interface!
  sig {abstract.void}
  def fx_currency
  end
end

module CardPaymentRecord
  include PaymentRecord
end

class Card
  include CardPaymentRecord
end
