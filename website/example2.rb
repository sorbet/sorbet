# typed: true

module AbstractAPIMethod
  extend T::Sig
  extend T::Helpers
  extend T::Generic
  abstract!

  InputType = type_member
  OutputType = type_member

  # Throws if parse fails
  sig {abstract.params(raw_input: T.untyped).returns(InputType)}
  def parse_input(raw_input); end

  sig {abstract.params(input: InputType).returns(OutputType)}
  def perform(input); end
end

class Charge < T::Struct
  const :id, String
  const :amount, Integer
  const :customer, String
end

class ChargeCreateAPIMethod
  extend T::Sig
  extend T::Generic

  extend AbstractAPIMethod

  class Input < T::Struct
    const :amount, Integer
    prop :customer, String
  end
  InputType = type_template(fixed: Input)
  OutputType = type_template#(fixed: Charge)

  sig {override.params(raw_input: T.untyped).returns(InputType)}
  def self.parse_input(raw_input)
    raise unless raw_input.is_a?(Hash)
    amount = raw_input.fetch('amount')
    customer = raw_input.fetch('customer')
    Input.new(amount: amount, customer: customer)
  end

  sig {override.params(input: InputType).returns(OutputType)}
  def self.perform(input)
    puts "Charging #{input.customer} amount of #{input.amount}"
    raise "unimplemented"
    # Charge.new(
    #   id: 'ch_123',
    #   amount: input.amount,
    #   customer: input.customer
    # )
  end
end

extend T::Sig
sig {params(method: AbstractAPIMethod[ChargeCreateAPIMethod::Input, Charge]).void}
def foo(method)
  T.reveal_type(method.perform(T.reveal_type(method.parse_input(nil))))
end

foo(ChargeCreateAPIMethod)
