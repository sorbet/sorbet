# typed: true

class A
  def outer_helper; end

  describe "inside describe" do
    def my_helper; end

    xit do
      my_helper
    end

    it "example", focus: true do
      my_helper
    end

    example do
      my_helper
    end
  end

  example do # error: Method `example` does not exist
    outer_helper # error: Method `outer_helper` does not exist
  end
end

module RSpec
  extend T::Sig

  sig { params(args: T.untyped, block: T.proc.void).void }
  def self.describe(*args, &block); end

  sig { params(args: T.untyped, block: T.proc.void).void }
  def self.xdescribe(*args, &block); end

  module Core
    class ExampleGroup
      def expect(*args); end
      def eq(*args); end
      def puts(*args); end
    end
  end
end

class RSpecLetBasicTest
  extend T::Sig

  RSpec.describe 'basic let functionality' do
    let(:foo) { 1 }

    before do
      foo
    end

    it 'creates a method from let' do
      result = foo
      puts result
    end

    it 'can use methods from RSpec::Core::ExampleGroup' do
      expect(1 + 1).to eq(2)
    end

    context "nested context" do
      let(:bar) { 2 }

      it 'creates a method from let in nested context' do
        result = bar
        puts result
      end
    end

    xcontext "pending context" do
      let(:baz) { 3 }

      it 'creates a method from let in pending context' do
        result = baz
        puts result
      end
    end
  end

  RSpec.xdescribe 'pending describe block' do
    let(:foo) { 1 }

    it 'creates a method from let' do
      result = foo
      puts result
    end
  end
end

