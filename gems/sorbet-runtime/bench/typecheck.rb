# frozen_string_literal: true
# typed: true

require 'benchmark'

require_relative '../lib/sorbet-runtime'

module SorbetBenchmarks
  module Typecheck
    extend T::Sig

    class Example; end

    def self.time_block(name, iterations_of_block: 1_000_000, iterations_in_block: 2, &blk)
      1_000.times(&blk) # warmup

      GC.start
      GC.disable

      t0 = Process.clock_gettime(Process::CLOCK_MONOTONIC)
      iterations_of_block.times(&blk)
      duration_s = Process.clock_gettime(Process::CLOCK_MONOTONIC) - t0

      GC.enable

      ns_per_iter = duration_s * 1_000_000_000 / (iterations_of_block * iterations_in_block)
      duration_str = ns_per_iter >= 1000 ? "#{(ns_per_iter / 1000).round(3)} μs" : "#{ns_per_iter.round(3)} ns"
      puts "#{name}: #{duration_str}"
    end

    def self.run
      example = Example.new

      time_block("Vanilla Ruby method call", iterations_in_block: 10) do
        unchecked_param(0)
        unchecked_param(1)
        unchecked_param(2)
        unchecked_param(3)
        unchecked_param(nil)
        unchecked_param(0)
        unchecked_param(1)
        unchecked_param(2)
        unchecked_param(3)
        unchecked_param(nil)
      end

      time_block("Vanilla Ruby is_a?", iterations_in_block: 10) do
        0.is_a?(Integer)
        1.is_a?(Integer)
        'str'.is_a?(Integer)
        nil.is_a?(Integer)
        false.is_a?(Integer)
        0.is_a?(Integer)
        1.is_a?(Integer)
        'str'.is_a?(Integer)
        nil.is_a?(Integer)
        false.is_a?(Integer)
      end

      time_block("T.unsafe", iterations_in_block: 10) do
        T.unsafe(0)
        T.unsafe(1)
        T.unsafe(2)
        T.unsafe(3)
        T.unsafe(nil)
        T.unsafe(0)
        T.unsafe(1)
        T.unsafe(2)
        T.unsafe(3)
        T.unsafe(nil)
      end

      time_block("T.must on non-nil", iterations_in_block: 10) do
        T.must(0)
        T.must(1)
        T.must(2)
        T.must(3)
        T.must(false)
        T.must(0)
        T.must(1)
        T.must(2)
        T.must(3)
        T.must(false)
      end

      type = T::Utils.coerce(Integer)
      time_block("T::Types::Simple#valid?", iterations_in_block: 10) do
        type.valid?(0)
        type.valid?(1)
        type.valid?(2)
        type.valid?(3)
        type.valid?(nil)
        type.valid?(0)
        type.valid?(1)
        type.valid?(2)
        type.valid?(3)
        type.valid?(nil)
      end

      type = T::Utils.coerce(T.nilable(Integer))
      time_block("T.nilable(Integer).valid?", iterations_in_block: 5) do
        type.valid?(0)
        type.valid?(1)
        type.valid?(2)
        type.valid?(nil)
        type.valid?(false)
      end

      type = T::Utils.coerce(T.any(Integer, Float, T::Boolean))
      time_block("T.any(Integer, Float, T::Boolean).valid?", iterations_in_block: 5) do
        type.valid?(0)
        type.valid?(1)
        type.valid?(2)
        type.valid?(nil)
        type.valid?('hi')
      end

      time_block("T.let(..., Integer)") do
        T.let(0, Integer)
        T.let(1, Integer)
      end

      time_block("sig {params(x: Integer).void}") do
        integer_param(0)
        integer_param(1)
      end

      my_proc = proc {}
      time_block("sig {params(x: Integer, blk: T.proc.void)} -- block literal") do
        integer_param_and_block(0) {}
        integer_param_and_block(0) {}
      end

      time_block("sig {params(x: Integer, blk: T.proc.void)} -- block pass") do
        integer_param_and_block(0, &my_proc)
        integer_param_and_block(0, &my_proc)
      end

      time_block("sig {params(x: Integer, blk: T.nilable(T.proc.void))} -- block literal") do
        integer_param_and_nilable_block(0) {}
        integer_param_and_nilable_block(0) {}
      end

      time_block("sig {params(x: Integer, blk: T.nilable(T.proc.void))} -- block pass") do
        integer_param_and_nilable_block(0, &my_proc)
        integer_param_and_nilable_block(0, &my_proc)
      end

      time_block("T.let(..., T.nilable(Integer))") do
        T.let(nil, T.nilable(Integer))
        T.let(1, T.nilable(Integer))
      end

      time_block("sig {params(x: T.nilable(Integer)).void}") do
        nilable_integer_param(nil)
        nilable_integer_param(1)
      end

      time_block("T.let(..., Example)") do
        T.let(example, Example)
        T.let(example, Example)
      end

      time_block("sig {params(x: Example).void}") do
        application_class_param(example)
        application_class_param(example)
      end

      time_block("sig {params(x: Example).returns(T.anything)}") do
        application_class_param_returns_anything(example)
        application_class_param_returns_anything(example)
      end

      time_block("T.let(..., T.nilable(Example))") do
        T.let(nil, T.nilable(Example))
        T.let(example, T.nilable(Example))
      end

      time_block("sig {params(x: T.nilable(Example)).void}") do
        nilable_application_class_param(nil)
        nilable_application_class_param(example)
      end

      time_block("sig {params(x: T.nilable(Example)).returns(T.anything)}") do
        nilable_application_class_param_returns_anything(nil)
        nilable_application_class_param_returns_anything(example)
      end

      time_block("sig {params(s: Symbol, x: Integer, y: Integer).void} (with kwargs)") do
        arg_plus_kwargs(:foo, x: 1, y: 2)
        arg_plus_kwargs(:bar, x: 1)
      end

      time_block("direct call Object#class") do
        example.class
        example.class
      end

      class_method = Object.instance_method(:class)
      time_block(".bind(example).call Object#class") do
        # rubocop:disable Performance/BindCall
        class_method.bind(example).call
        class_method.bind(example).call
        # rubocop:enable Performance/BindCall
      end

      if T::Configuration::AT_LEAST_RUBY_2_7
        time_block(".bind_call(example) Object#class") do
          class_method.bind_call(example)
          class_method.bind_call(example)
        end
      else
        puts 'skipping UnboundMethod#bind_call tests (re-run on Ruby 2.7+)'
      end
    end

    sig {params(x: Integer).void.checked(:never)}
    def self.unchecked_param(x); end

    sig {params(x: Integer).void}
    def self.integer_param(x); end

    sig {params(x: Integer, blk: T.proc.void).void}
    def self.integer_param_and_block(x, &blk); end

    sig {params(x: Integer, blk: T.nilable(T.proc.void)).void}
    def self.integer_param_and_nilable_block(x, &blk); end

    sig {params(x: T.nilable(Integer)).void}
    def self.nilable_integer_param(x); end

    sig {params(x: Example).void}
    def self.application_class_param(x); end

    sig {params(x: Example).returns(T.anything)}
    def self.application_class_param_returns_anything(x); end

    sig {params(x: T.nilable(Example)).void}
    def self.nilable_application_class_param(x); end

    sig {params(x: T.nilable(Example)).returns(T.anything)}
    def self.nilable_application_class_param_returns_anything(x); end

    sig {params(s: Symbol, x: Integer, y: Integer).void}
    def self.arg_plus_kwargs(s, x:, y: 0); end
  end
end
