# typed: true

module FinancingPeriod
  included do
    def starts_at_; end
  end
end

class FlexLoanRepaymentWindow
  include FinancingPeriod
end
