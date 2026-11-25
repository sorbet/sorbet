# typed: strict
# enable-experimental-rspec: true

module RSpec
  module Core
    class ExampleGroup
      extend T::Sig
    end
  end
end

RSpec.describe "inherited before hook" do
  before { raise "wrong message" unless error_message == "expected" }

  context "with a message" do
    sig {returns(String)}
    let(:error_message) { "expected" }

    it { 42 }
  end

  context "with another message" do
    sig {returns(String)}
    let(:error_message) { "expected" }

    it { 42 }
  end
end

RSpec.describe "per-example hook aliases" do
  before(:each) { error_message }
  before(:example) { error_message }
  after { error_message }
  after(:each) { error_message }
  after(:example) { error_message }

  context "with a message" do
    sig {returns(String)}
    let(:error_message) { "expected" }

    it { 42 }
  end
end

RSpec.describe "multiple inherited hooks" do
  before { first_message }
  before { second_message }

  context "intermediate" do
    before { third_message }

    context "leaf" do
      sig {returns(String)}
      let(:first_message) { "first" }

      sig {returns(String)}
      let(:second_message) { "second" }

      sig {returns(String)}
      let(:third_message) { "third" }

      it { 42 }
    end
  end
end

RSpec.describe "missing sibling hook dependency" do
  before { error_message } # error: Method `error_message` does not exist on `<describe 'missing sibling hook dependency'>::<context 'missing message'>`

  context "with a message" do
    sig {returns(String)}
    let(:error_message) { "expected" }

    it { 42 }
  end

  context "missing message" do
    it { 42 }
  end
end

RSpec.describe "parent hook consumption" do
  before { error_message } # error: Method `error_message` does not exist on `<describe 'parent hook consumption'>`

  it { 42 }

  context "with a message" do
    sig {returns(String)}
    let(:error_message) { "expected" }

    it { 42 }
  end
end

RSpec.describe "context hook receiver" do
  before(:context) { error_message } # error: Method `error_message` does not exist on `<describe 'context hook receiver'>`
  after(:all) { error_message } # error: Method `error_message` does not exist on `<describe 'context hook receiver'>`

  context "with a method" do
    sig {returns(String)}
    def error_message
      "expected"
    end

    it { 42 }
  end
end

RSpec.describe "context hook helper receiver" do
  sig {returns(String)}
  let(:prepared) { child_message } # error: Method `child_message` does not exist on `<describe 'context hook helper receiver'>`

  before(:context) { prepared }

  context "with a message" do
    sig {returns(String)}
    let(:child_message) { "expected" }

    it { prepared }
  end
end

RSpec.describe "opaque hooks remain conservative" do
  SCOPE = :example

  before(enabled: true) { error_message } # error: Method `error_message` does not exist on `<describe 'opaque hooks remain conservative'>`
  before(:each, :slow) { error_message } # error: Method `error_message` does not exist on `<describe 'opaque hooks remain conservative'>`
  before(SCOPE) { error_message } # error: Method `error_message` does not exist on `<describe 'opaque hooks remain conservative'>`
  before(:suite) { error_message } # error: Method `error_message` does not exist on `<describe 'opaque hooks remain conservative'>`

  context "with a message", enabled: true, slow: true do
    sig {returns(String)}
    let(:error_message) { "expected" }

    it { 42 }
  end
end

RSpec.describe "owner context helper" do
  sig {returns(String)}
  def error_message
    "expected"
  end

  before(:context) { error_message }

  context "child" do
    it { 42 }
  end
end
