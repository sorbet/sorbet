# typed: true

module FinancingPeriod
end

class FlexLoanRepaymentWindow
  include FinancingPeriod
  # included {...} blocks actually look like this at runtime
  # (no superclass method)
  def starts_at_; end

  # Sorbet's behavior on this case differs from the behavior of the
  # old, Ruby-powered version, because it had access to more runtime
  # reflection to decide whether to emit a method.
end
