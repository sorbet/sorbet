# typed: true

module Fooable
  extend T::Sig
  extend T::Helpers
  interface!

  sig {abstract.returns(T.nilable(String))}
  def foo; end
end

# This is incorrect: attr_accessor adds both a reader and a writer
class BadFooable
  extend T::Sig
  include Fooable

  sig {override.returns(T.nilable(String))}
  attr_accessor :foo # error: Method `BadFooable#foo=` is marked `override` but the parent only defines a writer method
end

# This is correct: Separate reader and writer
class GoodFooable
  extend T::Sig
  include Fooable

  sig {override.returns(T.nilable(String))}
  attr_reader :foo

  sig {params(foo: T.nilable(String)).void}
  attr_writer :foo
end

module WriterFooable
  extend T::Sig
  extend T::Helpers
  interface!

  sig {abstract.params(value: T.nilable(String)).void}
  def foo=(value); end
end

# This is incorrect: attr_accessor adds both a reader and a writer
class WriterOnlyBad # error: Missing definition for abstract method `WriterFooable#foo=` in `WriterOnlyBad`
  extend T::Sig
  include WriterFooable

  sig {override.returns(String)}
  attr_accessor :bar # error: Method `WriterOnlyBad#bar=` is marked `override` but the parent only defines a writer method
# ^^^^^^^^^^^^^^^^^^ error: Method `WriterOnlyBad#bar` is marked `override` but the parent only defines a reader method
end

module PropReadable
  extend T::Sig
  extend T::Helpers
  interface!

  sig {abstract.returns(T.nilable(String))}
  def prop_foo; end
end

# This is incorrect: It should error but doesn't yet because of a bug where props/const don't participate in override checking
class BadPropReadable
  extend T::Sig
  include T::Props
  include PropReadable
  
  prop :prop_foo, String # Should error: Method is marked `override` but the parent only defines a reader method
end

# This will be correct once props/const participate in override checking
class GoodPropReadable
  extend T::Sig
  include T::Props
  include PropReadable
  
  const :prop_foo, String
end

module PropWritable
  extend T::Sig
  extend T::Helpers
  interface!

  sig {abstract.params(value: String).void}
  def prop_foo=(value); end
end

class BadPropWritable
  extend T::Sig
  include T::Props
  include PropWritable
  
  prop :prop_foo, String # Should error: Method is marked `override` but the parent only defines a writer method
end

class GoodPropWritable
  extend T::Sig
  include T::Props
  include PropWritable
  
  prop :prop_foo, String, writer: :private
end
