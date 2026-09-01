# typed: true
# stratum: 1

module Base
  class Thing
    def secret
      P::Secret2.new
    end
  end
end
