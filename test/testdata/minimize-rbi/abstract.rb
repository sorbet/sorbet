# typed: strict

module PaymentRecord
  extend T::Sig
  extend T::Helpers
  interface!
  sig {abstract.void}
  def fx_currency
  end
end

module CardPaymentRecord # error: Missing definition for abstract method `PaymentRecord#fx_currency`
  include PaymentRecord
end

  class Card
# ^^^^^^^^^^ error: Missing definition for abstract method `PaymentRecord#fx_currency`
    include CardPaymentRecord
  end
