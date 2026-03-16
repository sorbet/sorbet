# typed: true
# enable-experimental-rbs-comments: true

# Data.define with RBS comment sig instead of T::Sig.
# The RBS rewriter translates `#:` into sig nodes before the Data rewriter runs,
# so typed accessors should work identically to the `sig { }` form.

RbsMoney = Data.define(:amount, :currency) do
  #: (amount: Numeric, currency: String) -> void
  def initialize(amount:, currency:) = super
end

module RbsDataChecks
  def self.test_typed_readers
    m = RbsMoney.new(amount: 10, currency: "CAD")
    T.assert_type!(m.amount, Numeric)
    T.assert_type!(m.currency, String)
  end

  def self.test_wrong_type
    RbsMoney.new(amount: "bad", currency: "CAD") # error: Expected `Numeric` but found `String("bad")` for argument `amount`
  end

  def self.test_missing_kwarg
    RbsMoney.new(amount: 10) # error: Missing required keyword argument `currency` for method `RbsMoney#initialize`
  end
end
