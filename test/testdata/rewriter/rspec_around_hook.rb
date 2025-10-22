# typed: true
# enable-experimental-rspec: true

# This test verifies that RSpec's `around` hook is properly supported.
# The `around` hook takes a parameter (the example) and requires explicit
# control over when the test runs via example.run

# Define minimal RSpec module for testing
module RSpec
  module Core
    class ExampleGroup
    end
  end

  def self.describe(description, &block); end
end

# Test RSpec.describe with around hooks
RSpec.describe "User" do
  let(:name) { "Alice" }
  let(:age) { 25 }

  before do
    name
  end

  around do |example|
    T.reveal_type(example) # error: Revealed type: `T.untyped`
    # Setup code before the test
    name
    # Run the test
    example.run
    # Teardown code after the test
    age
  end

  after do
    age
  end

  it "has a name and age" do
    name
    age
  end

  context "when verified" do
    let(:verified) { true }

    around do |spec|
      T.reveal_type(spec) # error: Revealed type: `T.untyped`
      # Nested around hook with different parameter name
      verified
      spec.run
      verified
    end

    it "shows verified status" do
      name
      age
      verified
    end
  end
end
