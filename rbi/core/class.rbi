# typed: __STDLIB_INTERNAL

# Extends any [`Class`](https://docs.ruby-lang.org/en/2.6.0/Class.html) to
# include *json\_creatable?* method.
# Classes in Ruby are first-class objects---each is an instance of class
# `Class`.
#
# Typically, you create a new class by using:
#
# ```ruby
# class Name
#  # some code describing the class behavior
# end
# ```
#
# When a new class is created, an object of type
# [`Class`](https://docs.ruby-lang.org/en/2.6.0/Class.html) is initialized and
# assigned to a global constant (`Name` in this case).
#
# When `Name.new` is called to create a new object, the `new` method in `Class`
# is run by default. This can be demonstrated by overriding `new` in `Class`:
#
# ```ruby
# class Class
#   alias old_new new
#   def new(*args)
#     print "Creating a new ", self.name, "\n"
#     old_new(*args)
#   end
# end
#
# class Name
# end
#
# n = Name.new
# ```
#
# *produces:*
#
# ```ruby
# Creating a new Name
# ```
#
# Classes, modules, and objects are interrelated. In the diagram that follows,
# the vertical arrows represent inheritance, and the parentheses metaclasses.
# All metaclasses are instances of the class 'Class'.
#
# ```
#                          +---------+             +-...
#                          |         |             |
#          BasicObject-----|-->(BasicObject)-------|-...
#              ^           |         ^             |
#              |           |         |             |
#           Object---------|----->(Object)---------|-...
#              ^           |         ^             |
#              |           |         |             |
#              +-------+   |         +--------+    |
#              |       |   |         |        |    |
#              |    Module-|---------|--->(Module)-|-...
#              |       ^   |         |        ^    |
#              |       |   |         |        |    |
#              |     Class-|---------|---->(Class)-|-...
#              |       ^   |         |        ^    |
#              |       +---+         |        +----+
#              |                     |
# obj--->OtherClass---------->(OtherClass)-----------...
# ```
class Class < Module
  # Allocates space for a new object of *class*'s class and does not call
  # initialize on the new instance. The returned object must be an instance of
  # *class*.
  #
  # ```ruby
  # klass = Class.new do
  #   def initialize(*args)
  #     @initialized = true
  #   end
  #
  #   def initialized?
  #     @initialized || false
  #   end
  # end
  #
  # klass.allocate.initialized? #=> false
  # ```
  sig {returns(T.untyped)}
  def allocate(); end

  ### Sorbet hijacks Class#new to re-use the sig from MyClass#initialize when creating new instances of a class.
  ### This method must be here so that all calls to MyClass.new aren't forced to take 0 arguments.

  # Calls `allocate` to create a new object of *class*'s class, then invokes
  # that object's `initialize` method, passing it *args*. This is the method
  # that ends up getting called whenever an object is constructed using .new.
  sig {params(args: T.untyped).returns(T.untyped)}
  def new(*args); end

  # Callback invoked whenever a subclass of the current class is created.
  #
  # Example:
  #
  # ```ruby
  # class Foo
  #   def self.inherited(subclass)
  #     puts "New subclass: #{subclass}"
  #   end
  # end
  #
  # class Bar < Foo
  # end
  #
  # class Baz < Bar
  # end
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # New subclass: Bar
  # New subclass: Baz
  # ```
  sig do
    params(
        arg0: Class,
    )
    .returns(T.untyped)
  end
  def inherited(arg0); end

  sig do
    params(
        arg0: T::Boolean,
    )
    .returns(T::Array[Symbol])
  end
  def instance_methods(arg0=T.unsafe(nil)); end

  sig {returns(T.nilable(String))}
  def name(); end

  # Returns the superclass of *class*, or `nil`.
  #
  # ```ruby
  # File.superclass          #=> IO
  # IO.superclass            #=> Object
  # Object.superclass        #=> BasicObject
  # class Foo; end
  # class Bar < Foo; end
  # Bar.superclass           #=> Foo
  # ```
  #
  # Returns nil when the given class does not have a parent class:
  #
  # ```ruby
  # BasicObject.superclass   #=> nil
  # ```
  sig {returns(T.nilable(Class))}
  def superclass(); end

  sig {void}
  sig do
    params(
        superclass: Class,
    )
    .void
  end
  sig do
    params(
        blk: T.proc.params(arg0: Class).returns(BasicObject),
    )
    .void
  end
  sig do
    params(
        superclass: Class,
        blk: T.proc.params(arg0: Class).returns(BasicObject),
    )
    .void
  end
  def initialize(superclass=Object, &blk); end
end
