# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class ValidateOverrideShapeTest < Critic::Unit::UnitTest
    class Base

      sig do
        overridable
        .params(req: Object, opt: Object, kwreq: Object, kwopt: Object, blk: Proc)
        .returns(Object)
      end
      def foo(req, opt=nil, kwreq:, kwopt: nil, &blk); end
    end

    it "succeeds if the override matches the shape" do
      klass = Class.new(Base) do
        sig do
          override
          .params(req: Object, opt: Object, kwreq: Object, kwopt: Object, blk: Proc)
          .returns(Object)
        end
        def foo(req, opt=nil, kwreq:, kwopt: nil, &blk); end
      end
      klass.new.foo(1, kwreq: 3) {}
    end

    it "succeeds if the override has additional optional args and kwargs" do
      klass = Class.new(Base) do
        sig do
          override
          .params(req: Object, opt: Object, opt2: Object, kwreq: Object, kwopt: Object, kwopt2: Object, blk: Proc)
          .returns(Object)
        end
        def foo(req, opt=nil, opt2=nil, kwreq:, kwopt: nil, kwopt2: nil, &blk); end
      end
      klass.new.foo(1, kwreq: 3) {}
    end

    it "succeeds if the override makes required parameters optional" do
      klass = Class.new(Base) do
        sig do
          override
          .params(req: Object, opt: Object, kwreq: Object, kwopt: Object, blk: Proc)
          .returns(Object)
        end
        def foo(req=nil, opt=nil, kwreq: nil, kwopt: nil, &blk); end
      end
      klass.new.foo(1, kwreq: 3) {}
    end

    it "raises if the override is missing a block arg" do
      klass = Class.new(Base) do
        sig do
          override
          .params(req: Object, opt: Object, kwreq: Object, kwopt: Object)
          .returns(Object)
        end
        def foo(req, opt=nil, kwreq:, kwopt: nil); end
      end
      err = assert_raises(RuntimeError) do
        klass.new.foo(1, kwreq: 3) {}
      end
      assert_includes(
        err.message,
        "Your definition of `foo` must accept a block parameter"
      )
      assert_includes(
        err.message,
        "in #{Base} at #{__FILE__}"
      )
    end

    it "raises if the override is missing positional args" do
      klass = Class.new(Base) do
        sig do
          override
          .params(req: Object, kwreq: Object, kwopt: Object, blk: Proc)
          .returns(Object)
        end
        def foo(req, kwreq:, kwopt: nil, &blk); end
      end
      err = assert_raises(RuntimeError) do
        klass.new.foo(1, kwreq: 3) {}
      end
      assert_includes(
        err.message,
        "Your definition of `foo` must accept at least 2 positional arguments"
      )
      assert_includes(
        err.message,
        "in #{Base} at #{__FILE__}"
      )
    end

    it "raises if the override has extra required args" do
      klass = Class.new(Base) do
        sig do
          override
          .params(req: Object, opt: Object, kwreq: Object, kwopt: Object, blk: Proc)
          .returns(Object)
        end
        def foo(req, opt, kwreq:, kwopt: nil, &blk); end
      end
      err = assert_raises(RuntimeError) do
        klass.new.foo(1, kwreq: 3) {}
      end
      assert_includes(
        err.message,
        "Your definition of `foo` must have no more than 1 required argument(s)"
      )
      assert_includes(
        err.message,
        "in #{Base} at #{__FILE__}"
      )
    end

    it "raises if the override is missing keyword args" do
      klass = Class.new(Base) do
        sig do
          override
          .params(req: Object, opt: Object, kwreq: Object, blk: Proc)
          .returns(Object)
        end
        def foo(req, opt=nil, kwreq:, &blk); end
      end
      err = assert_raises(RuntimeError) do
        klass.new.foo(1, kwreq: 3) {}
      end
      assert_includes(
        err.message,
        "Your definition of `foo` is missing these keyword arg(s): [:kwopt]"
      )
      assert_includes(
        err.message,
        "in #{Base} at #{__FILE__}"
      )
    end

    it "raises if the override has extra required keyword args" do
      klass = Class.new(Base) do
        sig do
          override
          .params(req: Object, opt: Object, kwreq: Object, kwopt: Object, blk: Proc)
          .returns(Object)
        end
        def foo(req, opt=nil, kwreq:, kwopt:, &blk); end
      end
      err = assert_raises(RuntimeError) do
        klass.new.foo(1, kwreq: 3) {}
      end
      assert_includes(
        err.message,
        "Your definition of `foo` has extra required keyword arg(s) [:kwopt]"
      )
      assert_includes(
        err.message,
        "in #{Base} at #{__FILE__}"
      )
    end

    it "succeeds if the override allows it" do
      klass = Class.new(Base) do
        sig do
          override(allow_incompatible: true)
          .returns(Object)
        end
        def foo; end
      end
      klass.new.foo
    end
  end
end
