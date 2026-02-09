# typed: true
# enable-experimental-rspec: true

# This test shows that RSpec DSL methods like `let`, `it`, and `context`
# are recognized by Sorbet with no errors.
# RSpec.describe accepts multiple arguments (metadata tags), and the first
# argument can be a string, class, symbol, or other type.

# Define minimal RSpec module for testing
module RSpec
  module Core
    class ExampleGroup
    end
  end

  def self.describe(description, *args, &block); end
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

# Test RSpec.describe with a string and a symbol metadata tag
RSpec.describe "User", :needs_macos do
  let(:path) { "/some/path" }

  it "has a path" do
    path
  end
end

# Test RSpec.describe with a string and multiple metadata tags
RSpec.describe "User", :needs_macos, :slow do
  let(:value) { 42 }

  it "has a value" do
    value
  end
end

# Test RSpec.describe with a string, symbol, and hash metadata
RSpec.describe "User", :needs_macos, focus: true do
  let(:count) { 10 }

  it "has a count" do
    count
  end
end

# Test RSpec.describe with a class as the first argument
class UserClass
end

RSpec.describe UserClass do
#              ^^^^^^^^^ hover: T.class_of(UserClass)
  let(:name) { "Bob" }

  it "has a name" do
    name
  end
end

# Test RSpec.describe with a class and metadata
RSpec.describe UserClass, :needs_macos do
#              ^^^^^^^^^ hover: T.class_of(UserClass)
  let(:value) { 100 }

  it "has a value" do
    value
  end
end

# Test RSpec.describe with a symbol as the first argument
RSpec.describe :user_symbol do
  let(:data) { "test" }

  it "has data" do
    data
  end
end

# Test RSpec.describe with a symbol and metadata
RSpec.describe :user_symbol, :slow do
  let(:result) { true }

  it "has a result" do
    result
  end
end
