# typed: __STDLIB_INTERNAL

# The marshaling library converts collections of Ruby objects into a byte
# stream, allowing them to be stored outside the currently active script. This
# data may subsequently be read and the original objects reconstituted.
#
# Marshaled data has major and minor version numbers stored along with the
# object information. In normal use, marshaling can only load data written with
# the same major version number and an equal or lower minor version number. If
# Ruby's "verbose" flag is set (normally using -d, -v, -w, or --verbose) the
# major and minor numbers must match exactly.
# [`Marshal`](https://docs.ruby-lang.org/en/2.6.0/Marshal.html) versioning is
# independent of Ruby's version numbers. You can extract the version by reading
# the first two bytes of marshaled data.
#
# ```ruby
# str = Marshal.dump("thing")
# RUBY_VERSION   #=> "1.9.0"
# str[0].ord     #=> 4
# str[1].ord     #=> 8
# ```
#
# Some objects cannot be dumped: if the objects to be dumped include bindings,
# procedure or method objects, instances of class
# [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html), or singleton objects, a
# [`TypeError`](https://docs.ruby-lang.org/en/2.6.0/TypeError.html) will be
# raised.
#
# If your class has special serialization needs (for example, if you want to
# serialize in some specific format), or if it contains objects that would
# otherwise not be serializable, you can implement your own serialization
# strategy.
#
# There are two methods of doing this, your object can define either
# marshal\_dump and marshal\_load or \_dump and \_load. marshal\_dump will take
# precedence over \_dump if both are defined. marshal\_dump may result in
# smaller [`Marshal`](https://docs.ruby-lang.org/en/2.6.0/Marshal.html) strings.
#
# ## Security considerations
#
# By design,
# [`Marshal.load`](https://docs.ruby-lang.org/en/2.6.0/Marshal.html#method-c-load)
# can deserialize almost any class loaded into the Ruby process. In many cases
# this can lead to remote code execution if the
# [`Marshal`](https://docs.ruby-lang.org/en/2.6.0/Marshal.html) data is loaded
# from an untrusted source.
#
# As a result,
# [`Marshal.load`](https://docs.ruby-lang.org/en/2.6.0/Marshal.html#method-c-load)
# is not suitable as a general purpose serialization format and you should never
# unmarshal user supplied input or other untrusted data.
#
# If you need to deserialize untrusted data, use
# [`JSON`](https://docs.ruby-lang.org/en/2.6.0/JSON.html) or another
# serialization format that is only able to load simple, 'primitive' types such
# as [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html),
# [`Array`](https://docs.ruby-lang.org/en/2.6.0/Array.html),
# [`Hash`](https://docs.ruby-lang.org/en/2.6.0/Hash.html), etc. Never allow user
# input to specify arbitrary types to deserialize into.
#
# ## marshal\_dump and marshal\_load
#
# When dumping an object the method marshal\_dump will be called. marshal\_dump
# must return a result containing the information necessary for marshal\_load to
# reconstitute the object. The result can be any object.
#
# When loading an object dumped using marshal\_dump the object is first
# allocated then marshal\_load is called with the result from marshal\_dump.
# marshal\_load must recreate the object from the information in the result.
#
# Example:
#
# ```ruby
# class MyObj
#   def initialize name, version, data
#     @name    = name
#     @version = version
#     @data    = data
#   end
#
#   def marshal_dump
#     [@name, @version]
#   end
#
#   def marshal_load array
#     @name, @version = array
#   end
# end
# ```
#
# ## \_dump and \_load
#
# Use \_dump and \_load when you need to allocate the object you're restoring
# yourself.
#
# When dumping an object the instance method \_dump is called with an
# [`Integer`](https://docs.ruby-lang.org/en/2.6.0/Integer.html) which indicates
# the maximum depth of objects to dump (a value of -1 implies that you should
# disable depth checking). \_dump must return a
# [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) containing the
# information necessary to reconstitute the object.
#
# The class method \_load should take a
# [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html) and use it to
# return an object of the same class.
#
# Example:
#
# ```ruby
# class MyObj
#   def initialize name, version, data
#     @name    = name
#     @version = version
#     @data    = data
#   end
#
#   def _dump level
#     [@name, @version].join ':'
#   end
#
#   def self._load args
#     new(*args.split(':'))
#   end
# end
# ```
#
# Since
# [`Marshal.dump`](https://docs.ruby-lang.org/en/2.6.0/Marshal.html#method-c-dump)
# outputs a string you can have \_dump return a
# [`Marshal`](https://docs.ruby-lang.org/en/2.6.0/Marshal.html) string which is
# Marshal.loaded in \_load for complex objects.
module Marshal
  # major version
  MAJOR_VERSION = T.let(T.unsafe(nil), Integer)
  # minor version
  MINOR_VERSION = T.let(T.unsafe(nil), Integer)

  # Serializes obj and all descendant objects. If anIO is specified, the
  # serialized data will be written to it, otherwise the data will be returned
  # as a [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html). If limit
  # is specified, the traversal of subobjects will be limited to that depth. If
  # limit is negative, no checking of depth will be performed.
  #
  # ```ruby
  # class Klass
  #   def initialize(str)
  #     @str = str
  #   end
  #   def say_hello
  #     @str
  #   end
  # end
  # ```
  #
  # (produces no output)
  #
  # ```ruby
  # o = Klass.new("hello\n")
  # data = Marshal.dump(o)
  # obj = Marshal.load(data)
  # obj.say_hello  #=> "hello\n"
  # ```
  #
  # [`Marshal`](https://docs.ruby-lang.org/en/2.6.0/Marshal.html) can't dump
  # following objects:
  # *   anonymous Class/Module.
  # *   objects which are related to system (ex:
  #     [`Dir`](https://docs.ruby-lang.org/en/2.6.0/Dir.html),
  #     [`File::Stat`](https://docs.ruby-lang.org/en/2.6.0/File/Stat.html),
  #     [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html),
  #     [`File`](https://docs.ruby-lang.org/en/2.6.0/File.html),
  #     [`Socket`](https://docs.ruby-lang.org/en/2.6.0/Socket.html) and so on)
  # *   an instance of
  #     [`MatchData`](https://docs.ruby-lang.org/en/2.6.0/MatchData.html),
  #     [`Data`](https://docs.ruby-lang.org/en/2.6.0/Data.html),
  #     [`Method`](https://docs.ruby-lang.org/en/2.6.0/Method.html),
  #     [`UnboundMethod`](https://docs.ruby-lang.org/en/2.6.0/UnboundMethod.html),
  #     [`Proc`](https://docs.ruby-lang.org/en/2.6.0/Proc.html),
  #     [`Thread`](https://docs.ruby-lang.org/en/2.6.0/Thread.html),
  #     [`ThreadGroup`](https://docs.ruby-lang.org/en/2.6.0/ThreadGroup.html),
  #     [`Continuation`](https://docs.ruby-lang.org/en/2.6.0/Continuation.html)
  # *   objects which define singleton methods
  sig do
    params(
        arg0: Object,
        arg1: IO,
        arg2: Integer,
    )
    .returns(Object)
  end
  sig do
    params(
        arg0: Object,
        arg1: Integer,
    )
    .returns(Object)
  end
  def self.dump(arg0, arg1=T.unsafe(nil), arg2=T.unsafe(nil)); end

  # Returns the result of converting the serialized data in source into a Ruby
  # object (possibly with associated subordinate objects). source may be either
  # an instance of [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) or an
  # object that responds to to\_str. If proc is specified, each object will be
  # passed to the proc, as the object is being deserialized.
  #
  # Never pass untrusted data (including user supplied input) to this method.
  # Please see the overview for further details.
  sig do
    params(
        arg0: T.any(String, IO),
        arg1: Proc,
    )
    .returns(Object)
  end
  def self.load(arg0, arg1=T.unsafe(nil)); end

  # Returns the result of converting the serialized data in source into a Ruby
  # object (possibly with associated subordinate objects). source may be either
  # an instance of [`IO`](https://docs.ruby-lang.org/en/2.6.0/IO.html) or an
  # object that responds to to\_str. If proc is specified, each object will be
  # passed to the proc, as the object is being deserialized.
  #
  # Never pass untrusted data (including user supplied input) to this method.
  # Please see the overview for further details.
  def self.restore(*_); end
end
