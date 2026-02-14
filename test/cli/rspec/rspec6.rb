# typed: true
# enable-experimental-rspec: true

# This test validates that various RSpec DSL methods properly support
# multiple arguments (like metadata tags).
#
# NOTE: This file is primarily for rewriter/snapshot coverage. It produces many
# "Method does not exist" errors when run WITHOUT --enable-experimental-rspec
# (expected). With the flag, the RSpec rewriter transforms the DSL; some errors
# may still appear due to the minimal RSpec stub below (e.g. `let`/`it` on the
# block receiver). The test shell runs Sorbet twice to capture both snapshots.
#
# The following RSpec methods commonly accept additional arguments:
# - it/specify/example (and x/f variants): description + metadata
# - before/after: scope (:each, :all) + optional metadata filters
# - shared_examples/shared_context: name + metadata
# - describe/context: description + metadata (multi-arg in rewriter; see also PR #9807)

# Define minimal RSpec module for testing
module RSpec
  module Core
    class ExampleGroup
    end
  end

  def self.describe(*args, &block); end
end

class MyClass
end

# =============================================================================
# Test: `it` with multiple arguments (description + metadata symbols/hashes)
# =============================================================================

# Single argument (baseline - should work)
RSpec.describe MyClass do
  let(:name) { "Alice" }

  it "has a name" do
    name
  end
end

# Multiple arguments: string + symbol metadata
RSpec.describe MyClass do
  let(:value) { 42 }

  it "has a value", :slow do
    value
  end
end

# Multiple arguments: string + multiple metadata symbols
RSpec.describe MyClass do
  let(:count) { 10 }

  it "has a count", :slow, :needs_macos do
    count
  end
end

# Multiple arguments: string + symbol + hash metadata
RSpec.describe MyClass do
  let(:result) { true }

  it "has a result", :slow, focus: true do
    result
  end
end

# =============================================================================
# Test: `specify` with multiple arguments
# =============================================================================

RSpec.describe MyClass do
  let(:data) { "test" }

  specify "something works", :unit do
    data
  end

  specify "another thing", :slow, :integration do
    data
  end
end

# =============================================================================
# Test: `example` with multiple arguments
# =============================================================================

RSpec.describe MyClass do
  let(:item) { "item" }

  example "demonstrates behavior", :smoke do
    item
  end

  example "shows feature", :regression, timeout: 10 do
    item
  end
end

# =============================================================================
# Test: `before` and `after` with scope arguments
# =============================================================================

RSpec.describe MyClass do
  let(:setup_value) { 1 }

  # before with :each scope (most common usage)
  before(:each) do
    setup_value
  end

  # before with :all scope
  before(:all) do
    setup_value
  end

  # after with :each scope
  after(:each) do
    setup_value
  end

  # after with :all scope
  after(:all) do
    setup_value
  end

  it "works" do
    setup_value
  end
end

# =============================================================================
# Test: `before` and `after` with metadata filters (RSpec filtering feature)
# See: https://rspec.info/features/3-13/rspec-core/hooks/filtering
# =============================================================================

RSpec.describe MyClass do
  let(:filtered_value) { "filtered" }

  # before with scope + symbol metadata filter
  # This runs only for examples tagged with :slow
  before(:each, :slow) do
    filtered_value
  end

  # before with scope + hash metadata filter
  before(:example, authorized: true) do
    filtered_value
  end

  # before with scope + multiple metadata filters
  before(:context, :integration, :needs_db) do
    filtered_value
  end

  # after with scope + symbol metadata filter
  after(:each, :slow) do
    filtered_value
  end

  # after with scope + hash metadata filter
  after(:example, cleanup: true) do
    filtered_value
  end

  # after with scope + multiple metadata filters
  after(:all, :integration, requires_cleanup: true) do
    filtered_value
  end

  it "works with filtered hooks", :slow do
    filtered_value
  end
end

# =============================================================================
# Test: `shared_examples` with metadata
# =============================================================================

RSpec.describe MyClass do
  shared_examples "reusable examples", :no_api do
    it "does something" do
      true
    end
  end

  include_examples "reusable examples"
end

# =============================================================================
# Test: `shared_context` with metadata  
# =============================================================================

RSpec.describe MyClass do
  shared_context "common setup", :needs_network do
    let(:connection) { "connected" }
  end

  include_context "common setup"

  it "uses the connection" do
    connection
  end
end

# =============================================================================
# Test: Nested context with multi-arg it
# =============================================================================

RSpec.describe MyClass do
  describe "nested tests" do
    let(:data) { "test" }

    it "has data", :integration do
      data
    end

    context "when something", :fast do
      it "still works", :unit, timeout: 5 do
        data
      end
    end
  end
end

# =============================================================================
# Test: `xit` (skipped it) with multiple arguments
# =============================================================================

RSpec.describe MyClass do
  let(:skipped) { "skipped" }

  xit "is pending", :slow do
    skipped
  end
end

# =============================================================================
# Test: `fit` (focused it) and `focus` with multiple arguments
# =============================================================================

RSpec.describe MyClass do
  let(:focused) { "focused" }

  fit "is focused", :critical do
    focused
  end

  focus "focused example", :tag do
    focused
  end
end

# =============================================================================
# Test: `xspecify` and `fspecify` with multiple arguments
# =============================================================================

RSpec.describe MyClass do
  let(:value) { 123 }

  xspecify "pending spec", :slow do
    value
  end

  fspecify "focused spec", :fast do
    value
  end
end

# =============================================================================
# Test: `xexample` and `fexample` with multiple arguments  
# =============================================================================

RSpec.describe MyClass do
  let(:sample) { "sample" }

  xexample "pending example", :slow do
    sample
  end

  fexample "focused example", :fast do
    sample
  end
end

# =============================================================================
# Test: `pending` and `skip` with multiple arguments
# =============================================================================

RSpec.describe MyClass do
  let(:item) { "item" }

  pending "not yet implemented", :future do
    item
  end

  skip "temporarily disabled", :broken do
    item
  end
end
