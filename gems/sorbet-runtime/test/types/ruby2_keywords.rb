# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::Ruby2KeywordsTest < Critic::Unit::UnitTest
  # Verifies that sig-wrapped methods do not emit ruby2_keywords warnings on
  # Ruby 3.4+, where the VM warns if ruby2_keywords is applied to a method that
  # already handles keywords or does not accept a splat.

  def warnings_for
    old_stderr = $stderr
    $stderr = StringIO.new
    yield
    $stderr.string
  ensure
    $stderr = old_stderr
  end

  it "does not warn for a plain method with no splat" do
    klass = Class.new do
      extend T::Sig
      sig { params(a: Integer, b: Integer).returns(Integer) }
      def add(a, b) = a + b
    end

    output = warnings_for { klass.new.add(1, 2) }
    assert_empty output.lines.grep(/ruby2_keywords/)
  end

  it "does not warn for a method with only keyword args" do
    klass = Class.new do
      extend T::Sig
      sig { params(a: Integer).returns(Integer) }
      def double(a:) = a * 2
    end

    output = warnings_for { klass.new.double(a: 5) }
    assert_empty output.lines.grep(/ruby2_keywords/)
  end

  it "does not warn for a method with splat and keyword args" do
    klass = Class.new do
      extend T::Sig
      sig { params(args: Integer, opts: String).returns(NilClass) }
      def mixed(*args, **opts) = nil
    end

    output = warnings_for { klass.new.mixed(1, 2, key: "val") }
    assert_empty output.lines.grep(/ruby2_keywords/)
  end

  it "does not warn for a method with only a splat" do
    klass = Class.new do
      extend T::Sig
      sig { params(args: Integer).returns(NilClass) }
      def splat_only(*args) = nil
    end

    output = warnings_for { klass.new.splat_only(1, 2, 3) }
    assert_empty output.lines.grep(/ruby2_keywords/)
  end

  it "correctly calls methods after sig wrapping" do
    klass = Class.new do
      extend T::Sig
      sig { params(a: Integer, b: Integer).returns(Integer) }
      def add(a, b) = a + b

      sig { params(args: Integer).returns(Integer) }
      def sum(*args) = args.sum

      sig { params(a: Integer).returns(Integer) }
      def double(a:) = a * 2
    end

    instance = klass.new
    assert_equal 3, instance.add(1, 2)
    assert_equal 6, instance.sum(1, 2, 3)
    assert_equal 10, instance.double(a: 5)
  end
end
