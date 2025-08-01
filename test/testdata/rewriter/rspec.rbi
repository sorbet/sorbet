# typed: strict

module RSpec
  sig {params(args: T.untyped, example_group_block: T.proc.bind(Core::ExampleGroup).void).void}
  def self.describe(*args, &example_group_block); end

  sig {params(args: T.untyped, example_group_block: T.proc.bind(Core::ExampleGroup).void).void}
  def self.xdescribe(*args, &example_group_block); end

  sig {params(args: T.untyped, example_group_block: T.proc.bind(Core::ExampleGroup).void).void}
  def self.context(*args, &example_group_block); end

  sig {params(name: String, args: T.untyped, example_group_block: T.proc.bind(Core::ExampleGroup).void).void}
  def self.shared_examples_for(name, *args, &example_group_block); end

  sig {params(name: String, args: T.untyped, example_group_block: T.proc.bind(Core::ExampleGroup).params(args: T.untyped).void).void}
  def self.shared_examples(name, *args, &example_group_block); end

  sig {params(name: String, args: T.untyped, example_group_block: T.proc.bind(Core::ExampleGroup).void).void}
  def self.shared_context(name, *args, &example_group_block); end

  def self.test_each(*args, &block); end

  module Core
    class ExampleGroup

      def self.test_each(*args, &block); end

      def test_each(*args, &block); end

      def include(*args); end

      def be_an_instance_of(*args); end

      def an_instance_of(*args); end

      def let(name, &block); end

      sig { returns(T.untyped) }
      sig { params(name: T.nilable(Symbol), block: T.proc.bind(T.untyped).void).returns(T.untyped) }
      def subject(name = nil, &block); end

      sig { params(block: T.proc.bind(RSpec::Core::ExampleGroup).void).returns(T.untyped) }
      sig { params(scope: Symbol, block: T.proc.bind(RSpec::Core::ExampleGroup).void).returns(T.untyped) }
      def before(scope = :each, &block); end

      sig { params(block: T.proc.params(arg0: RSpec::Core::ExampleGroup).bind(RSpec::Core::ExampleGroup).void).returns(T.untyped) }
      def around(&block); end

      sig { params(block: T.proc.bind(T.untyped).void).returns(T.untyped) }
      def after(&block); end

      sig { params(name: String, type: T.nilable(Symbol), block: T.proc.bind(RSpec::Core::ExampleGroup).void).returns(T.untyped) }
      def xit(name, type: nil, &block); end

      sig { params(block: T.proc.bind(RSpec::Core::ExampleGroup).void).returns(T.untyped) }
      sig { params(name: T.nilable(String), type: T.nilable(Symbol), block: T.proc.bind(RSpec::Core::ExampleGroup).void).returns(T.untyped) }
      def it(name = nil, type: nil, &block); end

      sig { params(block: T.proc.bind(RSpec::Core::ExampleGroup).void).returns(T.untyped) }
      sig { params(name: T.nilable(String), type: T.nilable(Symbol), block: T.proc.bind(RSpec::Core::ExampleGroup).void).returns(T.untyped) }
      def self.it(name = nil, type: nil, &block); end

      def context(name, **metadata, &block); end

      def pending(name, **metadata, &block); end

      def xcontext(name, &block); end

      def create(*args); end

      def build_list(*args); end

      def build_stubbed(*args); end

      def build(*args); end

      def expect(*args); end

      def allow(*args); end

      def instance_double(*args); end

      def class_double(*args); end

      def be_an(*args); end

      def all(*args); end

      def instance_of(*args); end

      def allow_any_instance_of(*args); end

      def having_attributes(*args); end

      sig { params(name: String, block: T.nilable(T.proc.bind(RSpec::Core::ExampleGroup).void)).returns(T.untyped) }
      def include_context(name, &block); end

      sig { params(name: String, block: T.proc.bind(RSpec::Core::ExampleGroup).void).returns(T.untyped) }
      def shared_examples(name, &block); end

      sig { params(name: String, block: T.proc.bind(RSpec::Core::ExampleGroup).void).returns(T.untyped) }
      def shared_context(name, &block); end

      def it_behaves_like(*args); end

      sig { params(name: T.nilable(T.any(T.any(String, Symbol), T::Array[T.any(String, Symbol)])), block: T.proc.bind(T.untyped).void).returns(T.untyped) }
      def its(name = nil, &block); end

      def expect(*args, &block); end
    end
  end
end

# Top-level DSL methods
sig {params(args: T.untyped, example_group_block: T.proc.bind(RSpec::Core::ExampleGroup).void).void}
def describe(*args, &example_group_block); end

sig {params(args: T.untyped, example_group_block: T.proc.bind(RSpec::Core::ExampleGroup).void).void}
def context(*args, &example_group_block); end
end
