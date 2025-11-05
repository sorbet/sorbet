# typed: true
# enable-experimental-rspec: true
# enable-experimental-requires-ancestor: true

module RSpec
  def self.context(arg0, &blk); end
  module Core
    class ExampleGroup
      def described_class
      end

      def is_expected
      end

      def expect(arg)
      end

      def eq(arg)
      end
    end
  end
end

class A
  def self.test_each(arg, &blk) = arg.each(&blk)

  def outer_helper; end

  def is_expected; end

  def expect(arg); end

  def eq(arg); end

  describe "inside describe" do
    def my_helper; end

    xit do
      my_helper
      described_class # error: does not exist
    end

    it "example", focus: true do
      my_helper
    end

    example do
      my_helper
    end

    shared_examples "some examples" do
      let(:defined_in_shared_examples) { "foo" }

      it("a shared example") do
        described_class
      end
    end

    describe "will include shared examples" do # error: `A::<describe 'inside describe'>::<describe 'will include shared examples'>` must inherit `RSpec::Core::ExampleGroup` (required by `A::<describe 'inside describe'>::<shared_examples 'some examples'>`)
      include_examples("some examples")

      it "has access to defined_in_shared_examples" do
        defined_in_shared_examples
      end
    end

    test_each([]) do |x|
      describe("shared examples in test_each") do
        include_examples("some examples")

        it "has access to defined_in_shared_examples" do
          defined_in_shared_examples # error: does not exist
        end
      end
    end
  end

  example do # error: Method `example` does not exist
    outer_helper # error: Method `outer_helper` does not exist
  end

  example_group "example_group group" do # error: Method `example_group` does not exist
    it do # error: does not exist
      outer_helper # error: Method `outer_helper` does not exist
    end
  end

  context "context group" do # error: Method `context` does not exist
    it do # error: does not exist
      outer_helper # error: Method `outer_helper` does not exist
    end
  end

  describe "contains generic name group" do
    example_group "example_group group" do
      it do
        outer_helper
      end
    end

    context "context group" do
      it do
        outer_helper
      end
    end
  end

  xdescribe "xdescribe group" do
    it do
      outer_helper
    end
  end

  describe "its support" do
    let(:foo) { "bar" }

    its(:bar) { is_expected.to eq(foo) } # error: Method `subject` does not exist on `A::<describe 'its support'>::<describe 'bar'>`

    # String arguments should work the same as symbols for simple attributes
    its('bar') { is_expected.to eq(foo) } # error: Method `subject` does not exist on `A::<describe 'its support'>::<describe 'bar'>`

    # Constants should be moved out of its blocks
    its(:size) do
      X = 1
      is_expected.to eq(X) # error: Method `subject` does not exist
    end

    # The correct desugaring is:
    #   its(:size) { is_expected.to eq(1) }
    # should generate:
    #   describe "size" do
    #     it "is_expected.to eq(1)" do
    #       expect(subject.size).to eq(1)
    #     end
    #   end
    its(:size) do
      T.reveal_type(self) # error: Revealed type: `A::<describe 'its support'>::<describe 'size'>`
    end
  end

  describe "its with typed subject" do
    class ThingWithSize
      extend T::Sig

      sig {returns(Integer)}
      def size
        42
      end
    end

    # Define subject using let with a signature
    extend T::Sig
    sig {returns(ThingWithSize)}
    let(:subject) { ThingWithSize.new }

    # The rewriter transforms `its(:size) { is_expected.to eq(42) }`
    # into `describe "size" do; it { expect(subject.size).to eq(42) }; end`
    # This allows proper type inference since subject is called directly
    its(:size) do
      # subject should be ThingWithSize (from the parent describe)
      T.reveal_type(subject) # error: Revealed type: `A::<describe 'its with typed subject'>::ThingWithSize`
      is_expected.to eq(42)
    end

    # This should error because no_such_method doesn't exist on ThingWithSize
    its(:no_such_method) do
      is_expected.to eq(0) # error: Method `no_such_method` does not exist on `A::<describe 'its with typed subject'>::ThingWithSize`
    end
  end
end

RSpec.context("B") do
  def another_outer_helper; end

  it "inside B" do
    another_outer_helper
  end

  context("Nested, no RSpec") do
    it "inside Nested" do
      another_outer_helper
      described_class
    end
  end
end
