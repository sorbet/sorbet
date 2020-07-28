# frozen_string_literal: true
# typed: true

require 'benchmark'

require_relative '../lib/sorbet-runtime'

module SorbetBenchmarks
  module Typecheck
    extend T::Sig

    class Example; end

    def self.run
      example = Example.new

      result = Benchmark.measure do
        1_000_000.times do
          T.let(0, Integer)
          T.let(1, Integer)
        end
      end
      puts "T.let(..., Integer) twice, μs/iter"
      puts result

      result = Benchmark.measure do
        1_000_000.times do
          integer_param(0)
          integer_param(1)
        end
      end
      puts "sig {params(x: Integer).void} twice, μs/iter"
      puts result

      result = Benchmark.measure do
        1_000_000.times do
          T.let(nil, T.nilable(Integer))
          T.let(1, T.nilable(Integer))
        end
      end
      puts "T.let(..., T.nilable(Integer)) twice, μs/iter"
      puts result

      result = Benchmark.measure do
        1_000_000.times do
          nilable_integer_param(nil)
          nilable_integer_param(1)
        end
      end
      puts "sig {params(x: T.nilable(Integer)).void} twice, μs/iter"
      puts result

      result = Benchmark.measure do
        1_000_000.times do
          T.let(example, Example)
          T.let(example, Example)
        end
      end
      puts "T.let(..., Example) twice, μs/iter"
      puts result

      result = Benchmark.measure do
        1_000_000.times do
          application_class_param(example)
          application_class_param(example)
        end
      end
      puts "sig {params(x: Example).void} twice, μs/iter"
      puts result

      result = Benchmark.measure do
        1_000_000.times do
          T.let(nil, T.nilable(Example))
          T.let(example, T.nilable(Example))
        end
      end
      puts "T.let(..., T.nilable(Example)) twice, μs/iter"
      puts result

      result = Benchmark.measure do
        1_000_000.times do
          nilable_application_class_param(nil)
          nilable_application_class_param(example)
        end
      end
      puts "sig {params(x: T.nilable(Example)).void} twice, μs/iter"
      puts result

      result = Benchmark.measure do
        1_000_000.times do
          arg_plus_kwargs(:foo, x: 1, y: 2)
          arg_plus_kwargs(:bar, x: 1)
        end
      end
      puts "sig {params(s: Symbol, x: Integer, y: Integer).void} (with kwargs) twice, μs/iter"
      puts result
    end

    sig {params(x: Integer).void}
    def self.integer_param(x); end

    sig {params(x: T.nilable(Integer)).void}
    def self.nilable_integer_param(x); end

    sig {params(x: Example).void}
    def self.application_class_param(x); end

    sig {params(x: T.nilable(Example)).void}
    def self.nilable_application_class_param(x); end

    sig {params(s: Symbol, x: Integer, y: Integer).void}
    def self.arg_plus_kwargs(s, x:, y: 0); end
  end
end
