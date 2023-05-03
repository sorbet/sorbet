# typed: __STDLIB_INTERNAL

# Extends any [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html) to
# include *json\_creatable?* method.
# Classes in Ruby are first-class objects---each is an instance of class
# [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html).
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
# [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html) is initialized and
# assigned to a global constant (Name in this case).
#
# When `Name.new` is called to create a new object, the
# [`new`](https://docs.ruby-lang.org/en/2.7.0/Class.html#method-i-new) method in
# [`Class`](https://docs.ruby-lang.org/en/2.7.0/Class.html) is run by default.
# This can be demonstrated by overriding
# [`new`](https://docs.ruby-lang.org/en/2.7.0/Class.html#method-i-new) in Class:
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
  extend T::Generic
  has_attached_class!(:out)

  ### TODO(jez) Use `T.attached_class` in `allocate`

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


  # Returns the object for which the receiver is the singleton class.
  #
  # Raises a `TypeError` if the class is not a singleton class.
  #
  # ```ruby
  # class Foo; end
  #
  # Foo.singleton_class.attached_object        #=> Foo
  # Foo.attached_object                        #=> TypeError: `Foo' is not a singleton class
  # Foo.new.singleton_class.attached_object    #=> #<Foo:0x000000010491a370>
  # TrueClass.attached_object                  #=> TypeError: `TrueClass' is not a singleton class
  # NilClass.attached_object                   #=> TypeError: `NilClass' is not a singleton class
  # ```
  sig { returns(BasicObject) }
  def attached_object; end

  ### Sorbet hijacks Class#new to re-use the sig from MyClass#initialize when creating new instances of a class.
  ### This method must be here so that all calls to MyClass.new aren't forced to take 0 arguments.

  # Calls
  # [`allocate`](https://docs.ruby-lang.org/en/2.7.0/Class.html#method-i-allocate)
  # to create a new object of *class*'s class, then invokes that object's
  # initialize method, passing it *args*. This is the method that ends up
  # getting called whenever an object is constructed using `.new`.
  sig {params(args: T.untyped, blk: T.untyped).returns(T.attached_class)}
  def new(*args, &blk); end

  # Creates a new anonymous (unnamed) class with the given superclass (or
  # Object if no parameter is given). You can give a class a name by assigning
  # the class object to a constant.
  #
  # If a block is given, it is passed the class object, and the block is
  # evaluated in the context of this class like class_eval.
  sig { params(blk: T.untyped).returns(T::Class[Object]) }
  sig do
    type_parameters(:Parent)
      .params(super_class: T.all(T::Class[T.type_parameter(:Parent)], T.type_parameter(:Parent)), blk: T.untyped)
      .returns(T.all(T::Class[T.type_parameter(:Parent)], T.type_parameter(:Parent)))
  end
  def self.new(super_class = Object, &blk); end

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

  # Returns an array of classes where the receiver is the direct superclass of
  # the class, excluding singleton classes. The order of the returned array is
  # not defined.
  #
  # ```ruby
  # class A; end
  # class B < A; end
  # class C < B; end
  # class D < A; end
  #
  # A.subclasses        #=> [D, B]
  # B.subclasses        #=> [C]
  # C.subclasses        #=> []
  # ```
  sig { returns(T::Array[Class]) }
  def subclasses(); end

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
