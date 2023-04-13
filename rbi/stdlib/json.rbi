# typed: __STDLIB_INTERNAL

# # JavaScript [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) Notation ([`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html))
#
# [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) is a lightweight
# data-interchange format. It is easy for us humans to read and write. Plus,
# equally simple for machines to generate or parse.
# [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) is completely language
# agnostic, making it the ideal interchange format.
#
# Built on two universally available structures:
#
# ```
# 1. A collection of name/value pairs. Often referred to as an _object_, hash table, record, struct, keyed list, or associative array.
# 2. An ordered list of values. More commonly called an _array_, vector, sequence or list.
# ```
#
# To read more about [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html)
# visit: http://json.org
#
# ## Parsing [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html)
#
# To parse a [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) string
# received by another application or generated within your existing application:
#
# ```ruby
# require 'json'
#
# my_hash = JSON.parse('{"hello": "goodbye"}')
# puts my_hash["hello"] => "goodbye"
# ```
#
# Notice the extra quotes `''` around the hash notation. Ruby expects the
# argument to be a string and can't convert objects like a hash or array.
#
# Ruby converts your string into a hash
#
# ## Generating [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html)
#
# Creating a [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) string for
# communication or serialization is just as simple.
#
# ```ruby
# require 'json'
#
# my_hash = {:hello => "goodbye"}
# puts JSON.generate(my_hash) => "{\"hello\":\"goodbye\"}"
# ```
#
# Or an alternative way:
#
# ```
# require 'json'
# puts {:hello => "goodbye"}.to_json => "{\"hello\":\"goodbye\"}"
# ```
#
# `JSON.generate` only allows objects or arrays to be converted to
# [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) syntax. `to_json`,
# however, accepts many Ruby classes even though it acts only as a method for
# serialization:
#
# ```
# require 'json'
#
# 1.to_json => "1"
# ```
module JSON
  FAST_STATE_PROTOTYPE = ::T.let(nil, ::T.untyped)
  Infinity = ::T.let(nil, ::T.untyped)
  JSON_LOADED = ::T.let(nil, ::T.untyped)
  MinusInfinity = ::T.let(nil, ::T.untyped)
  NaN = ::T.let(nil, ::T.untyped)
  PRETTY_STATE_PROTOTYPE = ::T.let(nil, ::T.untyped)
  SAFE_STATE_PROTOTYPE = ::T.let(nil, ::T.untyped)
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) version
  VERSION = ::T.let(nil, ::T.untyped)
  VERSION_ARRAY = ::T.let(nil, ::T.untyped)
  VERSION_BUILD = ::T.let(nil, ::T.untyped)
  VERSION_MAJOR = ::T.let(nil, ::T.untyped)
  VERSION_MINOR = ::T.let(nil, ::T.untyped)

  # If *object* is string-like, parse the string and return the parsed result as
  # a Ruby data structure. Otherwise generate a
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) text from the Ruby
  # data structure object and return it.
  #
  # The *opts* argument is passed through to generate/parse respectively. See
  # generate and parse for their documentation.
  sig do
    params(
      object: ::T.untyped,
      opts: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.[](object, opts=T.unsafe(nil)); end

  sig do
    params(
      modul: ::T.untyped,
      constant: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.const_defined_in?(modul, constant); end

  # This is create identifier, which is used to decide if the *json\_create*
  # hook of a class should be called. It defaults to 'json\_class'.
  sig {returns(::T.untyped)}
  def self.create_id(); end

  # This is create identifier, which is used to decide if the *json\_create*
  # hook of a class should be called. It defaults to 'json\_class'.
  sig do
    params(
      create_id: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.create_id=(create_id); end

  sig do
    params(
      path: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.deep_const_get(path); end

  sig do
    params(
      obj: ::T.untyped,
      anIO: ::T.untyped,
      limit: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.dump(obj, anIO=T.unsafe(nil), limit=T.unsafe(nil)); end

  # The global default options for the
  # [`JSON.dump`](https://docs.ruby-lang.org/en/2.7.0/JSON.html#method-i-dump)
  # method:
  #
  # ```
  # :max_nesting: false
  # :allow_nan: true
  # :allow_blank: true
  # ```
  sig {returns(::T.untyped)}
  def self.dump_default_options(); end

  # The global default options for the
  # [`JSON.dump`](https://docs.ruby-lang.org/en/2.7.0/JSON.html#method-i-dump)
  # method:
  #
  # ```
  # :max_nesting: false
  # :allow_nan: true
  # :allow_blank: true
  # ```
  sig do
    params(
      dump_default_options: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.dump_default_options=(dump_default_options); end

  sig do
    params(
      obj: ::T.untyped,
      opts: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.fast_generate(obj, opts=T.unsafe(nil)); end

  sig do
    params(
      obj: ::T.untyped,
      opts: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.fast_unparse(obj, opts=T.unsafe(nil)); end

  sig do
    params(
      obj: ::T.untyped,
      opts: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.generate(obj, opts=T.unsafe(nil)); end

  # Returns the [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html)
  # generator module that is used by
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html). This is either
  # [`JSON::Ext::Generator`](https://docs.ruby-lang.org/en/2.7.0/JSON/Ext/Generator.html)
  # or JSON::Pure::Generator.
  sig {returns(::T.untyped)}
  def self.generator(); end

  # Returns the [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html)
  # generator module that is used by
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html). This is either
  # [`JSON::Ext::Generator`](https://docs.ruby-lang.org/en/2.7.0/JSON/Ext/Generator.html)
  # or JSON::Pure::Generator.
  sig do
    params(
      generator: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.generator=(generator); end

  # Encodes string using Ruby's *String.encode*
  sig do
    params(
      to: ::T.untyped,
      from: ::T.untyped,
      string: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.iconv(to, from, string); end

  sig do
    params(
      source: ::T.untyped,
      proc: ::T.untyped,
      options: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.load(source, proc=T.unsafe(nil), options=T.unsafe(nil)); end

  # https://docs.ruby-lang.org/en/master/JSON.html#method-i-load_file
  sig do
    params(
      path: ::T.untyped,
      opts: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.load_file(path, opts=T.unsafe({})); end

  # https://docs.ruby-lang.org/en/master/JSON.html#method-i-load_file-21
  sig do
    params(
      path: ::T.untyped,
      opts: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.load_file!(path, opts=T.unsafe({})); end

  # The global default options for the
  # [`JSON.load`](https://docs.ruby-lang.org/en/2.7.0/JSON.html#method-i-load)
  # method:
  #
  # ```
  # :max_nesting: false
  # :allow_nan: true
  # :allow_blank: true
  # ```
  sig {returns(::T.untyped)}
  def self.load_default_options(); end

  # The global default options for the
  # [`JSON.load`](https://docs.ruby-lang.org/en/2.7.0/JSON.html#method-i-load)
  # method:
  #
  # ```
  # :max_nesting: false
  # :allow_nan: true
  # :allow_blank: true
  # ```
  sig do
    params(
      load_default_options: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.load_default_options=(load_default_options); end

  sig do
    params(
      json: String,
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.parse(json, args=T.unsafe(nil)); end

  sig do
    params(
      source: ::T.untyped,
      opts: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.parse!(source, opts=T.unsafe(nil)); end

  # Returns the [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) parser
  # class that is used by
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html). This is either
  # [`JSON::Ext::Parser`](https://docs.ruby-lang.org/en/2.7.0/JSON/Ext/Parser.html)
  # or JSON::Pure::Parser.
  sig {returns(::T.untyped)}
  def self.parser(); end

  # Returns the [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) parser
  # class that is used by
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html). This is either
  # [`JSON::Ext::Parser`](https://docs.ruby-lang.org/en/2.7.0/JSON/Ext/Parser.html)
  # or JSON::Pure::Parser.
  sig do
    params(
      parser: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.parser=(parser); end

  sig do
    params(
      obj: ::T.untyped,
      opts: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.pretty_generate(obj, opts=T.unsafe(nil)); end

  sig do
    params(
      obj: ::T.untyped,
      opts: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.pretty_unparse(obj, opts=T.unsafe(nil)); end

  sig do
    params(
      result: ::T.untyped,
      proc: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.recurse_proc(result, &proc); end

  # Alias for:
  # [`load`](https://docs.ruby-lang.org/en/2.7.0/JSON.html#method-i-load)
  sig do
    params(
      source: ::T.untyped,
      proc: ::T.untyped,
      options: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.restore(source, proc=T.unsafe(nil), options=T.unsafe(nil)); end

  # Returns the [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html)
  # generator state class that is used by
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html). This is either
  # [`JSON::Ext::Generator::State`](https://docs.ruby-lang.org/en/2.7.0/JSON/Ext/Generator/State.html)
  # or JSON::Pure::Generator::State.
  sig {returns(::T.untyped)}
  def self.state(); end

  # Returns the [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html)
  # generator state class that is used by
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html). This is either
  # [`JSON::Ext::Generator::State`](https://docs.ruby-lang.org/en/2.7.0/JSON/Ext/Generator/State.html)
  # or JSON::Pure::Generator::State.
  sig do
    params(
      state: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.state=(state); end

  sig do
    params(
      string: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.swap!(string); end

  sig do
    params(
      obj: ::T.untyped,
      opts: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.unparse(obj, opts=T.unsafe(nil)); end

  sig do
    params(
      obj: ::T.untyped,
      anIO: ::T.untyped,
      limit: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.unsafe_dump(obj, anIO=T.unsafe(nil), limit=T.unsafe(nil)); end

  sig do
    params(
      obj: ::T.untyped,
      opts: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.unsafe_generate(obj, opts=T.unsafe(nil)); end

  sig do
    params(
      source: ::T.untyped,
      opts: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.unsafe_parse(source, opts=T.unsafe(nil)); end

  sig do
    params(
      obj: ::T.untyped,
      opts: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.unsafe_pretty_generate(obj, opts=T.unsafe(nil)); end
end

class JSON::CircularDatastructure < JSON::NestingError
end

# This exception is raised if a generator or unparser error occurs.
class JSON::GeneratorError < JSON::JSONError
end

class JSON::GenericObject < OpenStruct
  sig do
    params(
      name: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def [](name); end

  sig do
    params(
      name: ::T.untyped,
      value: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def []=(name, value); end

  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def as_json(*_); end

  sig {returns(::T.untyped)}
  def to_hash(); end

  sig do
    params(
      a: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def to_json(*a); end

  sig do
    params(
      other: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def |(other); end

  sig do
    params(
      _: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.[](*_); end

  sig do
    params(
      obj: ::T.untyped,
      args: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.dump(obj, *args); end

  sig do
    params(
      object: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.from_hash(object); end

  sig do
    params(
      json_creatable: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.json_creatable=(json_creatable); end

  sig {returns(::T.untyped)}
  def self.json_creatable?(); end

  sig do
    params(
      data: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.json_create(data); end

  sig do
    params(
      source: ::T.untyped,
      proc: ::T.untyped,
      opts: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.load(source, proc=T.unsafe(nil), opts=T.unsafe(nil)); end
end

# This module holds all the modules/classes that implement JSON's functionality
# as C extensions.
module JSON::Ext
end

# This is the [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) generator
# implemented as a C extension. It can be configured to be used by setting
#
# ```ruby
# JSON.generator = JSON::Ext::Generator
# ```
#
# with the method generator= in
# [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html).
module JSON::Ext::Generator
end

module JSON::Ext::Generator::GeneratorMethods
end

module JSON::Ext::Generator::GeneratorMethods::Array
  # Returns a [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) string
  # containing a [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) array,
  # that is generated from this
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) instance. *state*
  # is a JSON::State object, that can also be used to configure the produced
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) string output
  # further.
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::String)
  end
  def to_json(*_); end
end

module JSON::Ext::Generator::GeneratorMethods::FalseClass
  # Returns a [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) string for
  # false: 'false'.
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::String)
  end
  def to_json(*_); end
end

module JSON::Ext::Generator::GeneratorMethods::Float
  # Returns a [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) string
  # representation for this
  # [`Float`](https://docs.ruby-lang.org/en/2.7.0/Float.html) number.
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::String)
  end
  def to_json(*_); end
end

module JSON::Ext::Generator::GeneratorMethods::Hash
  # Returns a [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) string
  # containing a [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) object,
  # that is generated from this
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) instance. *state* is
  # a JSON::State object, that can also be used to configure the produced
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) string output
  # further.
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::String)
  end
  def to_json(*_); end
end

module JSON::Ext::Generator::GeneratorMethods::Integer
  # Returns a [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) string
  # representation for this
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) number.
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::String)
  end
  def to_json(*_); end
end

module JSON::Ext::Generator::GeneratorMethods::NilClass
  # Returns a [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) string for
  # nil: 'null'.
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::String)
  end
  def to_json(*_); end
end

module JSON::Ext::Generator::GeneratorMethods::Object
  # Converts this object to a string (calling
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-to_s)),
  # converts it to a [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html)
  # string, and returns the result. This is a fallback, if no special method
  # to\_json was defined for some object.
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::String)
  end
  def to_json(*_); end
end

module JSON::Ext::Generator::GeneratorMethods::String
  # This string should be encoded with UTF-8 A call to this method returns a
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) string encoded with
  # UTF16 big endian characters as u????.
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::String)
  end
  def to_json(*_); end

  # This method creates a
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) text from the result
  # of a call to to\_json\_raw\_object of this
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::String)
  end
  def to_json_raw(*_); end

  # This method creates a raw object hash, that can be nested into other data
  # structures and will be generated as a raw string. This method should be
  # used, if you want to convert raw strings to
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) instead of UTF-8
  # strings, e. g. binary data.
  sig do
    returns(::T::Hash[T.untyped, T.untyped])
  end
  def to_json_raw_object; end

  # Extends *modul* with the String::Extend module.
  sig do
    params(
      _: ::T.untyped
    )
    .void
  end
  def self.included(_); end
end

module JSON::Ext::Generator::GeneratorMethods::String::Extend
  # Raw Strings are [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html)
  # Objects (the raw bytes are stored in an array for the key "raw"). The Ruby
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/JSON/Ext/Generator/GeneratorMethods/String.html)
  # can be created by this module method.
  sig do
    params(
      _: ::T::Hash[T.untyped, T.untyped]
    )
    .returns(::String)
  end
  def json_create(_); end
end

module JSON::Ext::Generator::GeneratorMethods::TrueClass
  # Returns a [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) string for
  # true: 'true'.
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::String)
  end
  def to_json(*_); end
end

class JSON::Ext::Generator::State
  sig do
    params(
      _: ::T.untyped
    )
    .void
  end
  def initialize(*_); end

  # Returns the value returned by method `name`.
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::T.untyped)
  end
  def [](_); end

  # Sets the attribute name to value.
  sig do
    params(
      name: ::T.untyped,
      value: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def []=(name, value); end

  # Returns true, if NaN, Infinity, and -Infinity should be generated, otherwise
  # returns false.
  sig do
    returns(::T.untyped)
  end
  def allow_nan?; end

  # This string is put at the end of a line that holds a
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) array.
  sig do
    returns(::T.untyped)
  end
  def array_nl; end

  # This string is put at the end of a line that holds a
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) array.
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::T.untyped)
  end
  def array_nl=(_); end

  # Returns true, if only ASCII characters should be generated. Otherwise
  # returns false.
  sig do
    returns(::T.untyped)
  end
  def ascii_only?; end

  # This integer returns the current initial length of the buffer.
  sig do
    returns(::T.untyped)
  end
  def buffer_initial_length; end

  # This sets the initial length of the buffer to `length`, if `length` > 0,
  # otherwise its value isn't changed.
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::T.untyped)
  end
  def buffer_initial_length=(_); end

  # Returns true, if circular data structures should be checked, otherwise
  # returns false.
  sig do
    returns(::T.untyped)
  end
  def check_circular?; end

  # Configure this
  # [`State`](https://docs.ruby-lang.org/en/2.7.0/JSON/Ext/Generator/State.html)
  # instance with the [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html)
  # *opts*, and return itself.
  #
  # Also aliased as:
  # [`merge`](https://docs.ruby-lang.org/en/2.7.0/JSON/Ext/Generator/State.html#method-i-merge)
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::T.untyped)
  end
  def configure(_); end

  # This integer returns the current depth of data structure nesting.
  sig do
    returns(::T.untyped)
  end
  def depth; end

  # This sets the maximum level of data structure nesting in the generated
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) to the integer
  # depth,
  # [`max_nesting`](https://docs.ruby-lang.org/en/2.7.0/JSON/Ext/Generator/State.html#method-i-max_nesting)
  # = 0 if no maximum should be checked.
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::T.untyped)
  end
  def depth=(_); end

  # Generates a valid [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html)
  # document from object `obj` and returns the result. If no valid
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) document can be
  # created this method raises a GeneratorError exception.
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::T.untyped)
  end
  def generate(_); end

  # Returns the string that is used to indent levels in the
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) text.
  sig do
    returns(::T.untyped)
  end
  def indent; end

  # Sets the string that is used to indent levels in the
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) text.
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::T.untyped)
  end
  def indent=(_); end

  # This integer returns the maximum level of data structure nesting in the
  # generated [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html),
  # [`max_nesting`](https://docs.ruby-lang.org/en/2.7.0/JSON/Ext/Generator/State.html#method-i-max_nesting)
  # = 0 if no maximum is checked.
  sig do
    returns(::T.untyped)
  end
  def max_nesting; end

  # This sets the maximum level of data structure nesting in the generated
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) to the integer
  # depth,
  # [`max_nesting`](https://docs.ruby-lang.org/en/2.7.0/JSON/Ext/Generator/State.html#method-i-max_nesting)
  # = 0 if no maximum should be checked.
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::T.untyped)
  end
  def max_nesting=(_); end

  # Alias for:
  # [`configure`](https://docs.ruby-lang.org/en/2.7.0/JSON/Ext/Generator/State.html#method-i-configure)
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::T.untyped)
  end
  def merge(_); end

  # This string is put at the end of a line that holds a
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) object (or
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html)).
  sig do
    returns(::T.untyped)
  end
  def object_nl; end

  # This string is put at the end of a line that holds a
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) object (or
  # [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html)).
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::T.untyped)
  end
  def object_nl=(_); end

  # Returns the string that is used to insert a space between the tokens in a
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) string.
  sig do
    returns(::T.untyped)
  end
  def space; end

  # Sets *space* to the string that is used to insert a space between the tokens
  # in a [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) string.
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::T.untyped)
  end
  def space=(_); end

  # Returns the string that is used to insert a space before the ':' in
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) objects.
  sig do
    returns(::T.untyped)
  end
  def space_before; end

  # Sets the string that is used to insert a space before the ':' in
  # [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) objects.
  sig do
    params(
      _: ::T.untyped
    )
    .returns(::T.untyped)
  end
  def space_before=(_); end

  # Returns the configuration instance variables as a hash, that can be passed
  # to the configure method.
  #
  # Also aliased as:
  # [`to_hash`](https://docs.ruby-lang.org/en/2.7.0/JSON/Ext/Generator/State.html#method-i-to_hash)
  sig do
    returns(::T.untyped)
  end
  def to_h; end

  # Alias for:
  # [`to_h`](https://docs.ruby-lang.org/en/2.7.0/JSON/Ext/Generator/State.html#method-i-to_h)
  sig do
    returns(::T.untyped)
  end
  def to_hash; end
end

# This is the [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html) parser
# implemented as a C extension. It can be configured to be used by setting
#
# ```ruby
# JSON.parser = JSON::Ext::Parser
# ```
#
# with the method parser= in
# [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html).
class JSON::Ext::Parser
  # Parses the current [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html)
  # text *source* and returns the complete data structure as a result.
  sig do
    returns(::T.untyped)
  end
  def parse; end

  # Returns a copy of the current *source* string, that was used to construct
  # this [`Parser`](https://docs.ruby-lang.org/en/2.7.0/JSON/Ext/Parser.html).
  sig do
    returns(::String)
  end
  def source; end
end


# The base exception for [`JSON`](https://docs.ruby-lang.org/en/2.7.0/JSON.html)
# errors.
class JSON::JSONError < StandardError
  sig do
    params(
      exception: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.wrap(exception); end
end

# This exception is raised if the required unicode support is missing on the
# system. Usually this means that the iconv library is not installed.
class JSON::MissingUnicodeSupport < JSON::JSONError
end

# This exception is raised if the nesting of parsed data structures is too deep.
class JSON::NestingError < JSON::ParserError
end

# This exception is raised if a parser error occurs.
class JSON::ParserError < JSON::JSONError
end
