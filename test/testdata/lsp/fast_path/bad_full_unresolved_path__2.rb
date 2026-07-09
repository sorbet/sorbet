# typed: strict

class Client
  include Interface

  extend T::Sig
  extend T::Helpers

  Proto = Interface::Proto

  sig do
    override
      .params(request: Proto::Response) # error: Unable to resolve constant `Response`
      .returns(Proto::Response) # error: Unable to resolve constant `Response`
  end
  def example(request)
    T.cast( # error: Please use `T.unsafe`
      T.unsafe(nil),
      Proto::Response # error: Unable to resolve constant `Response`
    )
  end
end
