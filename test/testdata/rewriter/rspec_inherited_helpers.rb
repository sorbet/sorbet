# typed: true
# enable-experimental-rspec: true

module RSpec
  module Core
    class ExampleGroup; end
  end
end

RSpec.describe "child-only let" do
  subject(:value) { input }
  #        ^^^^^ def: inherited-value

  context "with input" do
    let(:input) { "hello" }

    it "uses the child value" do
      value
    # ^^^^^ usage: inherited-value
    # ^^^^^ hover: T.untyped
    end
  end

  context "without using the subject" do
    it { 42 }
  end
end

class IntegerInputConsumer
  extend T::Sig

  sig {params(input: Integer).returns(String)}
  def call(input)
    input.to_s
  end
end

class StringInputConsumer
  extend T::Sig

  sig {params(input: String).returns(String)}
  def call(input)
    input
  end
end

RSpec.describe "independently typed siblings" do
  extend T::Sig

  PREFIX = "value: "

  sig {returns(String)}
  subject(:result) { PREFIX + handler.call(input) }

  context "integer input" do
    extend T::Sig

    sig {returns(Integer)}
    let(:input) { 42 }

    sig {returns(IntegerInputConsumer)}
    let(:handler) { IntegerInputConsumer.new }

    it do
      T.reveal_type(result) # error: Revealed type: `String`
    end
  end

  context "string input" do
    extend T::Sig

    sig {returns(String)}
    let(:input) { "hello" }

    sig {returns(StringInputConsumer)}
    let(:handler) { StringInputConsumer.new }

    it do
      T.reveal_type(result) # error: Revealed type: `String`
    end
  end
end

RSpec.describe "deep transitive helpers" do
  let(:prepared) { input }
  subject { prepared }

  def read_subject
    subject
  end

  before { prepared }

  context "intermediate" do
    context "with input" do
      let(:input) { "hello" }
      it { read_subject }
    end
  end
end

RSpec.describe "missing child input" do
  subject(:value) { input } # error: Method `input` does not exist on `<describe 'missing child input'>::<context 'missing'>`

  context "with input" do
    let(:input) { "hello" }
    it { value }
  end

  context "missing" do
    it { value }
  end
end

RSpec.describe "parent consumption" do
  subject(:value) { input } # error: Method `input` does not exist on `<describe 'parent consumption'>`

  it { value }

  context "with input" do
    let(:input) { "hello" }
    it { value }
  end
end

RSpec.describe "eager inherited helper" do
  let!(:value) { input } # error: Method `input` does not exist on `<describe 'eager inherited helper'>::<context 'missing'>`

  context "with input" do
    let(:input) { "hello" }
    it { 42 }
  end

  context "missing" do
    it { 42 }
  end
end

RSpec.describe "overridden helpers with super" do
  extend T::Sig

  sig {returns(String)}
  let(:value) { input }

  context "override" do
    extend T::Sig

    sig {override.returns(String)}
    let(:value) { super() + suffix }

    context "with dependencies" do
      extend T::Sig

      sig {returns(String)}
      let(:input) { "hello" }

      sig {returns(String)}
      let(:suffix) { "!" }

      it do
        T.reveal_type(value) # error: Revealed type: `String`
      end
    end
  end
end

RSpec.describe "invalid signed subject" do
  extend T::Sig

  sig {returns(String)}
  subject(:value) { input } # error: Expected `String` but found `Integer` for method result type

  context "integer input" do
    extend T::Sig

    sig {returns(Integer)}
    let(:input) { 42 }
    it { value }
  end
end

RSpec.describe "unused invalid helper" do
  let(:unused) { never_defined } # error: Method `never_defined` does not exist
  it { 42 }
end
