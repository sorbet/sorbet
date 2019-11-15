# typed: true
require "securerandom"

module SecureRandom; end

20_0000.times do
  SecureRandom.random_number(100)
end
