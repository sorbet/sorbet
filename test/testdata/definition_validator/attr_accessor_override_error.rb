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
  attr_accessor :foo # error: Method `BadFooable#foo=` is marked `override` but the parent only defines a reader method
end

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
  attr_accessor :bar # error: Method `WriterOnlyBad#bar=` is marked `override` but does not override anything
# ^^^^^^^^^^^^^^^^^^ error: Method `WriterOnlyBad#bar` is marked `override` but does not override anything
end

module PropReadable
  extend T::Sig
  extend T::Helpers
  interface!

  sig {abstract.returns(String)}
  def prop_foo; end
end

# This is incorrect: prop() generates both a getter and setter,
# which should conflict with a read-only interface.
# Currently won't error but *should* once props participate in override checking.
class BadPropReadable
  extend T::Sig
  include T::Props
  include PropReadable
  
  # Should be an error: "prop" introduces `prop_foo=`,
  # but the interface only defines `prop_foo` (reader).
  prop :prop_foo, String
end

class GoodPropReadable
  extend T::Sig
  include T::Props
  include PropReadable

  const :prop_foo, String
end
