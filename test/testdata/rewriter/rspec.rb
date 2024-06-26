# typed: true

class RSpec
  def self.describe(thing, &blk); end
end

class RSpec::Core::ExampleGroup
  def self.include_context(context)
  end
end

RSpec.describe('my group') do
  extend T::Sig
  include_context 'bar'

  def some_method
  end

  def self.let(sym, &blk); end

  let(:some_let) { 0 }

  it 'thing' do
    some_method
    some_let
  end
end
