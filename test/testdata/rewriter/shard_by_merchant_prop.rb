# typed: true

module Opus::Autogen::Tokens
  # This isn't exactly right but it's close
  AccountModelMerchantToken = T.type_alias {String}
end

module ShardByMerchant
  def merchant_prop(name: :merchant, override: false, without_accessors: false); end
end

module ShardByMerchantBase
  def merchant_token_prop(name: :merchant, override: false, without_accessors: false); end
end

class MerchantPropModel
  include T::Props
  extend ShardByMerchant

  merchant_prop
end

class MerchantTokenPropModel
  include T::Props
  extend ShardByMerchantBase

  merchant_token_prop
end

T.reveal_type(MerchantPropModel.new.merchant) # error: Revealed type: `String`
MerchantPropModel.new.merchant = "hi" # error: Method `merchant=` does not exist

T.reveal_type(MerchantTokenPropModel.new.merchant) # error: Revealed type: `T.untyped`
MerchantTokenPropModel.new.merchant = nil # error: Method `merchant=` does not exist
