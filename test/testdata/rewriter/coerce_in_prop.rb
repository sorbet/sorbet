# typed: true

# This used to be valid syntax. We only keep it around to guarantee that Sorbet
# doesn't crash on it. Any reasonable behvior is fine.
  class Inputs
    PAYMENT_METHODS_HASH = {a: Integer}
    def self.prop(sym, type, opts={}); end;

    prop :supported_payment_methods1, T.coerce(PAYMENT_METHODS_HASH) # error-with-dupes: Unsupported method `T.coerce`
    prop :supported_payment_methods2, T.coerce({a: Integer})
  end
