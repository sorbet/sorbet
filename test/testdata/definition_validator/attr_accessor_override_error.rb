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
  attr_accessor :foo # error: Method `BadFooable#foo=` is marked `override` but the interface only defines a reader method
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
class WriterOnlyBad
  extend T::Sig
  include WriterFooable

  sig {override.params(value: T.nilable(String)).void} # error: Unknown argument name `value`
  attr_accessor :foo # error: Method `WriterOnlyBad#foo` is marked `override` but the interface only defines a writer method
end
