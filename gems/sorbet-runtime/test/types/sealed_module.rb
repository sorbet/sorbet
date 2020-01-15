# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::SealedModuleTest < Critic::Unit::UnitTest

  # Some tests in this file need to require fixtures to exercise cross-file behavior.
  # The understanding is that all fixtures should put their classes into the same namespace,
  # so that we can clean up after them.

  before do
    module Opus::Types::Test::SealedModuleSandbox; end
  end
  after do
    Opus::Types::Test.send(:remove_const, :SealedModuleSandbox)
    T::Configuration.sealed_violation_whitelist = nil
  end

  it "allows declaring a class as sealed" do
    Class.new do
      extend T::Helpers
      sealed!
    end
  end

  it "allows declaring a module as sealed" do
    Module.new do
      extend T::Helpers
      sealed!
    end
  end

  it "allows inheriting a sealed class in the same file" do
    parent = Class.new do
      extend T::Helpers
      sealed!
    end
    Class.new(parent)
  end

  it "forbids inheriting a sealed class in another file" do
    require_relative './fixtures/sealed_module/forbid_sealed_class__1.rb'

    err = assert_raises(RuntimeError) do
      require_relative './fixtures/sealed_module/forbid_sealed_class__2.rb'
    end
    assert_match(/was declared sealed and can only be inherited in/, err.message)

    err = assert_raises(RuntimeError) do
      require_relative './fixtures/sealed_module/forbid_sealed_class__3.rb'
    end
    assert_match(/was declared sealed and can only be inherited in/, err.message)
  end

  it "allows including a sealed module in the same file" do
    m = Module.new do
      extend T::Helpers
      sealed!
    end
    Class.new do
      include m
    end
  end

  it "forbids including a sealed module in another file" do
    require_relative './fixtures/sealed_module/forbid_sealed_module_include__1.rb'

    err = assert_raises(RuntimeError) do
      require_relative './fixtures/sealed_module/forbid_sealed_module_include__2.rb'
    end
    assert_match(/was declared sealed and can only be included in/, err.message)

    err = assert_raises(RuntimeError) do
      require_relative './fixtures/sealed_module/forbid_sealed_module_include__3.rb'
    end
    assert_match(/was declared sealed and can only be included in/, err.message)
  end

  it "allows extending a sealed module in the same file" do
    m = Module.new do
      extend T::Helpers
      sealed!
    end
    Class.new do
      extend m
    end
  end

  it "forbids extending a sealed module in another file" do
    require_relative './fixtures/sealed_module/forbid_sealed_module_extend__1.rb'

    err = assert_raises(RuntimeError) do
      require_relative './fixtures/sealed_module/forbid_sealed_module_extend__2.rb'
    end
    assert_match(/was declared sealed and can only be extended in/, err.message)

    err = assert_raises(RuntimeError) do
      require_relative './fixtures/sealed_module/forbid_sealed_module_extend__3.rb'
    end
    assert_match(/was declared sealed and can only be extended in/, err.message)
  end

  it "forbids declaring a class as final and then sealed" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Helpers
        final!
        sealed!
      end
    end
    assert_match(/was already declared `final!` and cannot be declared `sealed!`/, err.message)
  end

  it "forbids declaring a class as sealed and then final" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Helpers
        sealed!
        final!
      end
    end
    assert_match(/was already declared as sealed and cannot be declared as final$/, err.message)
  end

  it "forbids re-declaring a class as sealed" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Helpers
        sealed!
        sealed!
      end
    end
    assert_match(/was already declared `sealed!` and cannot be re-declared `sealed!`/, err.message)
  end

  it "forbids declaring a non-module, non-method as final" do
    err = assert_raises(RuntimeError) do
      Array.new(1) do
        extend T::Helpers
        sealed!
      end
    end
    assert_match(/is not a class or module and cannot be declared `sealed!`$/, err.message)
  end

  it "finds the right file names with classes that are abstract! & sealed!" do
    # This test used to not work because both abstract! and sealed! change the inherited hook,
    # which adds extra lines to the backtrace (via super).
    require_relative './fixtures/sealed_module/sealed_abstract__1.rb'

    err = assert_raises(RuntimeError) do
      require_relative './fixtures/sealed_module/sealed_abstract__2.rb'
    end
    assert_match(/was declared sealed and can only be inherited in/, err.message)

    err = assert_raises(RuntimeError) do
      require_relative './fixtures/sealed_module/sealed_abstract__3.rb'
    end
    assert_match(/was declared sealed and can only be inherited in/, err.message)
  end

  it "allows whitelisting certain sealed violations" do
    require_relative './fixtures/sealed_module/whitelist_violation__1.rb'
    require_relative './fixtures/sealed_module/whitelist_violation__2.rb'
  end
end
