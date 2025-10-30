# typed: true
class Module; include T::Sig; end

class View
  extend T::Generic

  DataViewModelMember = type_member { {fixed: T.untyped} }

  sig { returns(T.nilable(T::Hash[T.untyped, T.untyped])) }
  attr_reader :inputs

  sig { returns(T.nilable(DataViewModelMember)) }
  private def instance
    inputs&.fetch(:model, nil)
  end

  sig { params(model: T.nilable(DataViewModelMember)).void }
  private def takes_member(model)
    T.reveal_type(model) # error: `T.untyped`
  end
end

class PaymentAttempt; end
class Model::PaymentIntent

  sig { returns(PaymentAttempt) }
  def current_attempt_ = PaymentAttempt.new
end

class PaymentIntent < View
  DataViewModelMember = type_member { {fixed: Model::PaymentIntent} }

  sig { returns(T.nilable(PaymentAttempt)) }
  def current_payment_attempt_
    T.reveal_type(instance) # error: `T.nilable(Model::PaymentIntent)`
    instance&.current_attempt_
  end
end
