# typed: true

module FinancingPeriod
end

class FlexLoanRepaymentWindow
  include FinancingPeriod
  # included {...} blocks actually look like this at runtime
  # (no superclass method)
  def starts_at_; end
end
