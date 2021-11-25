# typed: strict

module PaymentRecord
  extend T::Helpers
  def fx_currency
  end
end

module CardPaymentRecord
  include PaymentRecord
end

class Card
  include CardPaymentRecord
  def fx_currency; end
end
