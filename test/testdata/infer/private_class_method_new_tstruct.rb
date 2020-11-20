# typed: true

class A < T::Struct
  extend T::Sig
  private_class_method :new

  prop :must_be_even, Integer

  sig {params(must_be_even: Integer).returns(T.nilable(T.attached_class))}
  def self.make(must_be_even:)
    return nil unless must_be_even.even?
    new(must_be_even: must_be_even)
  end
end

A.new(must_be_even: 0) # error: Non-private call to private method `new`

T.reveal_type(A.make(must_be_even: 1)) # error: Revealed type: `T.nilable(A)`
