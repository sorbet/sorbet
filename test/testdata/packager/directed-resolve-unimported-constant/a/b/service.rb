# typed: true
# stratum: 1

module A # error: Package `A::B` may not open `A`
  module B
    class Service
      def run
        NeatFeature.new.only_on_a_neat_feature # error: Method `only_on_a_neat_feature` does not exist
      end
    end
  end
end
