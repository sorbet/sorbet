# typed: true
  class Inputs
    PAYMENT_METHODS_HASH = {a: Integer}
    def self.prop(sym, type); end;

    prop :supported_payment_methods1, T.coerce(PAYMENT_METHODS_HASH)
    prop :supported_payment_methods2, T.coerce({a: Integer})
  end
