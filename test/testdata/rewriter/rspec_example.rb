# typed: true
# enable-experimental-requires-ancestor: true

module RSpec
  module Core
    class ExampleGroup
      def described_class
      end
    end
  end
end

RSpec.describe("A") do
  def self.test_each(arg, &blk) = arg.each(&blk)

  def outer_helper; end

  describe "inside describe" do
    def my_helper; end

    xit do
      my_helper
      described_class
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

    describe "will include shared examples" do
      include_examples("some examples") # error: does not exist

      it "has access to defined_in_shared_examples" do
        defined_in_shared_examples # error: does not exist
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

  example do
    outer_helper
  end
end

RSpec.context("B") do
  def outer_helper; end

  it "inside B" do
    outer_helper
  end

  context("Nested, no RSpec") do
    it "inside Nested" do
      outer_helper
    end
  end
end
