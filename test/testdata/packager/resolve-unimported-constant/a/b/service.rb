# typed: true


module A # error: Package `A::B` may not open `A`
  module B
    class Service
      def run
        NeatFeature.new.only_on_a_neat_feature # error: `A` is not imported
      end
    end
  end
end
