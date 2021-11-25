# typed: true

module FinancingPeriod
  def self.included; end
  included do
    def starts_at_; end
  end
end

class FlexLoanRepaymentWindow
  include FinancingPeriod
end
