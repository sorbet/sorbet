# compiled: true
# typed: true

module Opus
  module Account
    module Model
      module Merchant
        
      end
    end
  end
end

module Opus::Autogen::Tokens::AccountModelMerchant
  # This isn't exactly right; this would actually be more like a type alias to
  # a String, but we're using a class to make sure that the Prop.cc logic is
  # right.
  class Token; end
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

T.reveal_type(MerchantTokenPropModel.new.merchant) # error: Revealed type: `Opus::Autogen::Tokens::AccountModelMerchant::Token`
MerchantTokenPropModel.new.merchant = nil # error: Method `merchant=` does not exist
