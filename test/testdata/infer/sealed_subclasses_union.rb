# typed: true

class ::Module
  include T::Sig
end

module Payment
  extend T::Helpers
  sealed!
  abstract!

  sig {abstract.returns(T.any(String, Integer))}
  def amount; end

  class Canceled < T::Struct
    include Payment

    const :amount, T.any(String, Integer)
  end

  class Open < T::Struct
    include Payment

    const :amount, T.any(String, Integer)
  end

  class Pending < T::Struct
    include Payment

    const :amount, T.any(String, Integer, Float)
  end
end
 
sig {params(p: T.nilable(Payment)).void}
def handle_payment(p)
  return unless p

  T.reveal_type(p) # error: T.any(Payment::Open, Payment::Pending, Payment::Canceled)
  T.reveal_type(p.amount) # error: T.any(Float, String, Integer)
end
