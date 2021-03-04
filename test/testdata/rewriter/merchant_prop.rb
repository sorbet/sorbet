# typed: true

class MyModel
  include T::Props
  def self.merchant_prop(opts={}); end

  # Opus::Account::Model::Merchant is in pay-server
  merchant_prop # error: Unable to resolve constant `Account`
end
