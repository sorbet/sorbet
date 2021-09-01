# typed: true

module ShardByMerchant
  def merchant_prop(name: :merchant, override: false, without_accessors: false); end
end

class MerchantPropModel
  include T::Props
  extend ShardByMerchant

  merchant_prop
end

T.reveal_type(MerchantPropModel.new.merchant) # error: Revealed type: `String`
MerchantPropModel.new.merchant = "hi" # error: Method `merchant=` does not exist

