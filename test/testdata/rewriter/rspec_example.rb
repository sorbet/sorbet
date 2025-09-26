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

RSpec.describe("A") do # error: does not exist
  def self.test_each(arg, &blk) = arg.each(&blk)

  def outer_helper; end

  describe "inside describe" do # error: does not exist
    def my_helper; end

    xit do # error: does not exist
      my_helper
      described_class # error: does not exist
    end

    it "example", focus: true do # error: does not exist
      my_helper
    end

    example do # error: does not exist
      my_helper
    end

    shared_examples "some examples" do # error: does not exist
      let(:defined_in_shared_examples) { "foo" } # error: does not exist

      it("a shared example") do # error: does not exist
        described_class # error: does not exist
      end
    end

    describe "will include shared examples" do # error: does not exist
      include_examples("some examples") # error: does not exist

      it "has access to defined_in_shared_examples" do # error: does not exist
        defined_in_shared_examples # error: does not exist
      end
    end

    test_each([]) do |x|
      describe("shared examples in test_each") do # error: does not exist
        include_examples("some examples") # error: does not exist

        it "has access to defined_in_shared_examples" do # error: does not exist
          defined_in_shared_examples # error: does not exist
        end
      end
    end
  end

  example do # error: does not exist
    outer_helper
  end
end

RSpec.context("B") do # error: does not exist
  def outer_helper; end

  it "inside B" do # error: does not exist
    outer_helper
  end

  context("Nested, no RSpec") do # error: does not exist
    it "inside Nested" do # error: does not exist
      outer_helper
    end
  end
end
