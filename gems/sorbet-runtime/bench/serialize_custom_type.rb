# frozen_string_literal: true
# typed: true

require 'benchmark'

require_relative '../lib/sorbet-runtime'

module SorbetBenchmarks
  module SerializeCustomType
    class MyCustomType
      extend T::Props::CustomType

      attr_accessor :value

      def initialize(value)
        @value = value
      end

      # Not used in this benchmark.
      def self.deserialize(value)
        result = new
        result.value = value.clone.freeze
        result
      end

      def self.serialize(instance)
        instance.value
      end
    end

    def self.run
      input = MyCustomType.new(123)

      100_000.times do
        T::Props::CustomType.checked_serialize(input)
      end

      result = Benchmark.measure do
        1_000_000.times do
          T::Props::CustomType.checked_serialize(input)
        end
      end

      puts "T::Props::CustomType.checked_serialize (μs/iter):"
      puts result
    end
  end
end
