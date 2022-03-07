# typed: true

# This used to be valid syntax. We only keep it around to guarantee that Sorbet
# doesn't crash on it. Any reasonable behvior is fine.
  class Inputs
    PAYMENT_METHODS_HASH = {a: Integer}
    include T::Props

    prop :supported_payment_methods1, T.coerce(PAYMENT_METHODS_HASH)
    #                                 ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unsupported method `T.coerce`
    #                                 ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unsupported method `T.coerce`
    #                                 ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unsupported method `T.coerce`
    #                                   ^^^^^^ error: Method `coerce` does not exist on `T.class_of(T)`
    #                                   ^^^^^^ error: Method `coerce` does not exist on `T.class_of(T)`
    #                                   ^^^^^^ error: Method `coerce` does not exist on `T.class_of(T)`
    #                                   ^^^^^^ error: Method `coerce` does not exist on `T.class_of(T)`
    prop :supported_payment_methods2, T.coerce({a: Integer})
    #                                 ^^^^^^^^^^^^^^^^^^^^^^ error: Unsupported method `T.coerce`
    #                                 ^^^^^^^^^^^^^^^^^^^^^^ error: Unsupported method `T.coerce`
    #                                 ^^^^^^^^^^^^^^^^^^^^^^ error: Unsupported method `T.coerce`
    #                                   ^^^^^^ error: Method `coerce` does not exist on `T.class_of(T)`
    #                                   ^^^^^^ error: Method `coerce` does not exist on `T.class_of(T)`
    #                                   ^^^^^^ error: Method `coerce` does not exist on `T.class_of(T)`
    #                                   ^^^^^^ error: Method `coerce` does not exist on `T.class_of(T)`
  end
