  class Inputs
    PAYMENT_METHODS_HASH = {a: Integer}

    optional :supported_payment_methods1, T.coerce(PAYMENT_METHODS_HASH)
    optional :supported_payment_methods2, T.coerce({a: Integer})
  end
