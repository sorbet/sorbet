# typed: true
# enable-experimental-requires-ancestor: true

class RSpec::Core::ExampleGroup
  def expect(val); end
  def eq(arg); end
  def subject; end
  def is_expected; end
  def to(matcher); end
end

def test_each(arr, &blk); end

RSpec.describe("shared example with no context") do
  shared_examples "my shared example" do
    let(:foo) { 'bar' }

    it "works" do
      expect(foo).to eq('bar')
    end
  end
end

RSpec.describe("single shared example with context") do
  shared_examples "my shared example" do
    let(:foo) { 'bar' }

    context 'context' do
      it 'works' do
        expect(foo).to eq('bar')
      end
    end
  end
end

RSpec.describe("multiple contexts") do
  shared_examples "complex processing" do
    let(:x) { 10 }

    context 'first context' do
      it "first test" do
        expect(x).to eq(10)
      end
    end

    context 'second context' do
      it "second test" do
        expect(x).to eq(10)
      end
    end
  end
end

RSpec.describe("nested contexts") do
  shared_examples "shared example with nested context" do
    let(:value) { 42 }

    context 'outer' do
      context 'inner' do
        it "nested test" do
          expect(value).to eq(42)
        end
      end
    end
  end
end

RSpec.describe("nested lets in nested context") do
  shared_examples "mixed" do
    let(:a) { 1 }
    let(:b) { 2 }

    context 'calculation' do
      let(:c) { 3 }

      it "calculates" do
        expect(a + b + c).to eq(6)
      end
    end
  end
end

RSpec.describe('nested shared examples with include examples') do
  shared_examples 'level 1' do
    let(:level1_var) { 'level1' }

    shared_examples 'level 2' do
      let(:level2_var) { 'level2' }

      it 'can access its own level' do
        expect(level2_var).to eq(level2_var)
      end

      it 'cannot access level 1 variable' do
        level_1_var # error: Method `level_1_var` does not exist
        level1_var # error: Method `level1_var` does not exist
      end
    end

    it 'can access its own level' do
      expect(level1_var).to eq(level1_var)
    end

    include_examples 'level 2'

    it 'can access a level 2 variable after include' do
      expect(level2_var).to eq(level2_var)
    end
  end

  include_examples 'level 1'

  it 'can access level 1 variable after include' do
    expect(level1_var).to eq(level1_var)
  end
end

RSpec.describe('including examples within a test_each block') do
  shared_examples 'shared in test_each' do
    let(:shared_var) { 'shared' }

    it 'can access shared_var' do
      expect(shared_var).to eq('shared')
    end
  end

  # NOTE: include_examples within describe blocks inside test_each is not yet supported.
  # The describe block doesn't get transformed into a proper class, so the include_examples
  # doesn't work correctly and shared_var is not accessible.
  test_each([1, 2, 3]) do |num|
    describe "with number #{num}" do
      include_examples 'shared in test_each'

      it 'can access shared_var' do
        expect(shared_var).to eq('shared') # error: Method `shared_var` does not exist
      end
    end
  end
end

RSpec.describe('its blocks inside shared examples') do
  subject { { name: 'test', value: 42 } }

  shared_examples "has attributes" do
    # its() creates a describe block for each attribute test
    # These need to inherit from RSpec::Core::ExampleGroup when inside a module
    its(:name) { is_expected.to eq('test') }
    its(:value) { is_expected.to eq(42) }
  end

  include_examples "has attributes"
end

RSpec.shared_context 'standalone shared context' do
  let(:foo) { 'bar' }

  it 'works' do
    expect(foo).to eq('bar')
  end

  context 'my context' do
    it 'works' do
      expect(foo).to eq('bar') # error: Method `foo` does not exist
    end
  end
end

RSpec.describe 'including standalone context' do
  include_context 'standalone shared context'

  it 'can use the let' do
    foo
  end
end

RSpec.shared_context 'standalone shared context with nested shared examples' do
  let(:parent_var) { 'value' }

  shared_examples "nested shared example" do
    let(:foo) { 'bar' }

    it 'works' do
      expect(foo).to eq('bar')
    end

    let(:attempted_parent_access) { parent_var } # error: Method `parent_var` does not exist

    it 'cannot access parent_var in it block either' do
      expect(parent_var).to eq('value') # error: Method `parent_var` does not exist on `<shared_examples 'standalone shared context with nested shared examples'>::<shared_examples 'nested shared example'>`
    end
  end
end

# Using the nested shared_examples from a describe block
RSpec.describe 'describe using nested shared example from standalone shared context' do
  include_context 'standalone shared context with nested shared examples'
  include_examples "nested shared example"

  it 'can use both' do
    foo
    parent_var
  end
end


RSpec.describe 'shared example block params' do
  shared_examples 'with params' do |param1, param2|
    it 'can access params' do
      # We intentionally type these as `T.untyped` for now but in the future
      # maybe we can give them better types.
      T.reveal_type(param1) # error: Revealed type: `T.untyped`
      T.reveal_type(param2) # error: Revealed type: `T.untyped`
    end
  end

  include_examples 'with params', 'value1', 'value2'
end


# Test that block parameters work in string interpolation
RSpec.describe 'shared example block param used in context description' do
  shared_examples "example with block param" do |param|
    T.reveal_type(param) # error: Revealed type: `T.untyped`
    it "has flag #{param}" do
      expect(param).to eq(1)
    end
  end

  include_examples "example with block param", "my_param"
end

RSpec.shared_examples 'standalone shared example' do
  let(:foo) { 'bar' }

  it 'works' do
    expect(foo).to eq('bar')
  end

  context 'my context' do
    it 'works' do
      expect(foo).to eq('bar') # error: Method `foo` does not exist
    end
  end
end

RSpec.describe 'describe using standalone shared example' do
  include_examples 'standalone shared example'

  it 'can use the let' do
    foo
  end
end
