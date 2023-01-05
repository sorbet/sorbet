# frozen_string_literal: true
# typed: false

# This file contains runtime stubs to make `T::Sig::WithoutRuntime.sig` work in
# the runtime. We're using `T::Sig::WithoutRuntime.sig` in this gem because we
# don't want to have to depend on the sorbet-runtime gem.

# Note that in particular because sigs are lazily evaluated and
# `T::Sig::WithoutRuntime.sig` never forces the sig block, we don't actually
# need implementations for anything that can only go inside the sig block.

module T
  module Sig
    module WithoutRuntime
      def self.sig(arg=nil, &blk); end
    end
  end

  def self.any(type_a, type_b, *types); end
  def self.nilable(type); end
  def self.untyped; end
  def self.noreturn; end
  def self.all(type_a, type_b, *types); end
  def self.enum(values); end
  def self.proc; end
  def self.self_type; end
  def self.class_of(klass); end
  def self.type_alias(type=nil, &blk); end
  def self.type_parameter(name); end

  def self.cast(value, type, checked: true); value; end
  def self.let(value, type, checked: true); value; end
  def self.bind(value, type, checked: true); value; end
  def self.assert_type!(value, type, checked: true); value; end
  def self.unsafe(value); value; end
  def self.must(arg, msg=nil); arg; end
  def self.reveal_type(value); value; end

  module Array
    def self.[](type); end
  end

  module Hash
    def self.[](keys, values); end
  end

  module Enumerable
    def self.[](type); end
  end

  module Enumerator
    def self.[](type); end

    module Lazy
      def self.[](type); end
    end

    module Chain
      def self.[](type); end
    end
  end

  module Range
    def self.[](type); end
  end

  module Set
    def self.[](type); end
  end
end
