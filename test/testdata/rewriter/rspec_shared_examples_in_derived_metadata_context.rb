# typed: true
# enable-experimental-rspec: true

module RSpec
  module Core
    class ExampleGroup
    end
  end

  def self.configure
    yield self
  end
  def self.include_context(_name, _tag); end
  def self.define_derived_metadata(*); yield({}); end
end

# Bare `shared_examples` defined inside an `RSpec.shared_context` whose tag is
# auto-applied via `RSpec.configure { config.include_context 'NAME', :TAG;
# config.define_derived_metadata(...) { |m| m[:TAG] = true } }` should be hoisted to
# root scope so consumers that pick up the outer context via derived_metadata (rather
# than an explicit lexical `include_context`) can still resolve them via
# `it_behaves_like` / `include_examples`.
#
# This mirrors zenpayroll's `packs/technical_services/auth/authorization/spec/support/
# shared_contexts/admin_abilities.rb` pattern, where a sibling `RSpec.configure`
# block wires the outer context to every spec under a file_path regex.

RSpec.shared_context 'outer auto included', :auto_tag do
  shared_examples 'an action that requires permission' do |action, permission|
    it 'works' do
    end
  end

  shared_examples 'an unauthorized action' do |action|
    it 'fails' do
    end
  end

  shared_examples_for 'a permitted operation' do |operation|
    it 'is permitted' do
    end
  end

  shared_context 'a nested context' do
    let(:foo) { 42 }
  end
end

RSpec.configure do |config|
  config.include_context 'outer auto included', :auto_tag
  config.define_derived_metadata(file_path: %r{/some/path/}) do |metadata|
    metadata[:auto_tag] = true
  end
end

# Consumer with no lexical `include_context` — at runtime RSpec applies the include via
# `define_derived_metadata`. The hoisted root-scoped synthetic modules let Sorbet
# resolve the references without needing a `RSpec.shared_examples` RBI shim.
RSpec.describe 'metadata-driven consumer' do
  it_behaves_like 'an action that requires permission', :create, :read
  it_behaves_like 'an unauthorized action', :delete
  include_examples 'a permitted operation', :update
  include_context 'a nested context'
end

# `include_context 'name' do ... end` (with a block) — RSpec runtime adds the block
# body as consumer-local setup on top of the include. The rewriter previously
# returned nullptr on this shape (dropping the include entirely); now we emit the
# include and ignore the block body, so the consumer's class still has the outer
# context in its ancestor chain for resolving sibling `it_behaves_like` references.
RSpec.shared_context 'with block context' do
  shared_examples 'block-form sibling examples' do
    it 'sibling works' do
    end
  end
end

RSpec.describe 'with-block include_context consumer' do
  include_context 'with block context' do
    let(:override) { :consumer_local }
  end
  it_behaves_like 'block-form sibling examples'
end

# Negative case: a tag that is NOT paired with `define_derived_metadata` should
# leave its nested shared_examples enclosing-scoped (the post-#10273 default).
# A consumer without an explicit `include_context` should NOT resolve the nested
# names — error expected.
RSpec.shared_context 'outer not auto included', :manual_only_tag do
  shared_examples 'manually included only' do
    it 'works only when explicitly included' do
    end
  end
end

RSpec.describe 'consumer that does not include outer' do
  it_behaves_like 'manually included only'
  #               ^^^^^^^^^^^^^^^^^^^^^^^^ error-with-dupes: Unable to resolve constant `<shared_examples 'manually included only'>`
end

# -----------------------------------------------------------------------------
# TODO(sorbet/sorbet): consumer-of-consumer chains across `include_context` ----
# -----------------------------------------------------------------------------
#
# When an outer `RSpec.shared_context` uses `include_context 'inner'` to pull in
# another shared_context, AND defines a nested `shared_examples` that references
# the inner's nested `shared_examples` via `include_examples`, the inner
# reference fails to resolve.
#
# At runtime RSpec inlines the inner context's body into the outer at each
# include site, so the inner's nested shared_examples gets re-registered in the
# outer's example group. Sorbet currently models `include_context` as a plain
# Ruby `include`, and Ruby's constant lookup does NOT walk a lexical parent's
# includes for inherited nested constants -- the same lookup fails in plain
# Ruby too:
#
#     module Outer; module Inner; end; end
#     class Container
#       include Outer
#       class Nested
#         p Inner   # raises uninitialized constant Container::Nested::Inner
#       end
#     end
#
# To fix: teach the rewriter that `include_context 'name'` inside an
# `RSpec.shared_context` body should inline the included context's nested
# `shared_examples` definitions (or otherwise expose them at root scope at the
# include site), rather than emitting a plain `include`. Mirrors zenpayroll's
# `packs/product_services/payroll/pufferfish/spec/support/shared_behaviors/
# individual_eft_debit.rb` pattern.

RSpec.shared_context 'inner ctx' do
  shared_examples 'inner shared examples' do
    it 'inner works' do
    end
  end
end

RSpec.shared_context 'outer ctx that includes inner' do
  include_context 'inner ctx'

  shared_examples 'outer shared examples that consumes inner' do
    it 'outer works' do
    end

    # TODO(sorbet/sorbet): should resolve via runtime inline-body semantics.
    include_examples 'inner shared examples'
    #                ^^^^^^^^^^^^^^^^^^^^^^^ error-with-dupes: Unable to resolve constant `<shared_examples 'inner shared examples'>`
  end
end

