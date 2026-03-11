# typed: true
# enable-experimental-rspec: true

# This test validates that `expect` can be called with zero positional arguments
# (block-only form), which is used for testing that code raises/does not raise errors.
# Example: `expect { some_code }.to raise_error`
#          `expect { some_code }.not_to raise_error`

module RSpec
  module Core
    class ExampleGroup
      # https://github.com/rspec/rspec/blob/rspec-expectations-v3.13.5/rspec-expectations/lib/rspec/expectations/syntax.rb#L72-L74
      def expect(value = nil, &block)
      end

      def raise_error
      end

      def eq(arg)
      end
    end
  end

  def self.describe(*args, &block); end
end

class MyClass
  def self.delete(key)
  end

  def self.write(key, value)
  end
end

RSpec.describe MyClass do
  # Block-only form: expect { }.not_to raise_error
  it "does not raise on delete" do
    expect { MyClass.delete(:bar) }
  end

  # Block-only form: expect { }.to raise_error
  it "raises on invalid input" do
    expect { MyClass.write(nil, nil) }
  end

  # Mixed: one describe with both forms
  describe "expect forms" do
    it "supports value form" do
      result = MyClass.delete(:foo)
      expect(result)
    end

    it "supports block form" do
      expect { MyClass.delete(:baz) }
    end
  end
end
