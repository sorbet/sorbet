# frozen_string_literal: true
# typed: true
# compiled: true

class Currency
  def self.supported
    ['usd', 'jpy', 'cad']
  end

  supported.each do |currency|
    define_singleton_method(currency) {currency}
  end
end

p T.unsafe(Currency).usd
p T.unsafe(Currency).jpy
p T.unsafe(Currency).cad
