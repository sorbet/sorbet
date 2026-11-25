# typed: true

# This test shows that RSpec DSL methods like `let`, `it`, and `context`
# are recognized by Sorbet with no errors.

# Define minimal RSpec module for testing
module RSpec
  module Core
    class ExampleGroup
    end
  end

  def self.describe(*args, &block); end
end

# Test RSpec.describe with let, it, and nested context
RSpec.describe "User" do
  let(:name) { "Alice" }

  it "has a name" do
    name
  end

  context "when verified" do
    let(:verified) { true }

    it "shows verified status" do
      name
      verified
    end
  end
end
