# typed: __STDLIB_INTERNAL

# [`BasicObject`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html) is the
# parent class of all classes in Ruby. It's an explicit blank class.
#
# [`BasicObject`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html) can be
# used for creating object hierarchies independent of Ruby's object hierarchy,
# proxy objects like the
# [`Delegator`](https://docs.ruby-lang.org/en/2.7.0/Delegator.html) class, or
# other uses where namespace pollution from Ruby's methods and classes must be
# avoided.
#
# To avoid polluting
# [`BasicObject`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html) for
# other users an appropriately named subclass of
# [`BasicObject`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html) should
# be created instead of directly modifying BasicObject:
#
# ```ruby
# class MyObjectSystem < BasicObject
# end
# ```
#
# [`BasicObject`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html) does not
# include [`Kernel`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html) (for
# methods like `puts`) and
# [`BasicObject`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html) is
# outside of the namespace of the standard library so common classes will not be
# found without using a full class path.
#
# A variety of strategies can be used to provide useful portions of the standard
# library to subclasses of
# [`BasicObject`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html). A
# subclass could `include Kernel` to obtain `puts`, `exit`, etc. A custom
# Kernel-like module could be created and included or delegation can be used via
# [`method_missing`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html#method-i-method_missing):
#
# ```ruby
# class MyObjectSystem < BasicObject
#   DELEGATE = [:puts, :p]
#
#   def method_missing(name, *args, &block)
#     return super unless DELEGATE.include? name
#     ::Kernel.send(name, *args, &block)
#   end
#
#   def respond_to_missing?(name, include_private = false)
#     DELEGATE.include?(name) or super
#   end
# end
# ```
#
# Access to classes and modules from the Ruby standard library can be obtained
# in a [`BasicObject`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html)
# subclass by referencing the desired constant from the root like `::File` or
# `::Enumerator`. Like
# [`method_missing`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html#method-i-method_missing),
# const\_missing can be used to delegate constant lookup to `Object`:
#
# ```ruby
# class MyObjectSystem < BasicObject
#   def self.const_missing(name)
#     ::Object.const_get(name)
#   end
# end
# ```
class BasicObject
  # Boolean negate.
  sig {returns(T::Boolean)}
  def !(); end

  # Returns true if two objects are not-equal, otherwise false.
  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def !=(other); end

  # Equality --- At the
  # [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) level, #==
  # returns `true` only if `obj` and `other` are the same object. Typically,
  # this method is overridden in descendant classes to provide class-specific
  # meaning.
  #
  # Unlike #==, the
  # [`equal?`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html#method-i-equal-3F)
  # method should never be overridden by subclasses as it is used to determine
  # object identity (that is, `a.equal?(b)` if and only if `a` is the same
  # object as `b`):
  #
  # ```ruby
  # obj = "a"
  # other = obj.dup
  #
  # obj == other      #=> true
  # obj.equal? other  #=> false
  # obj.equal? obj    #=> true
  # ```
  #
  # The eql? method returns `true` if `obj` and `other` refer to the same hash
  # key. This is used by [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html)
  # to test members for equality. For any pair of objects where eql? returns
  # `true`, the hash value of both objects must be equal. So any subclass that
  # overrides eql? should also override hash appropriately.
  #
  # For objects of class
  # [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html), eql?  is
  # synonymous with #==. Subclasses normally continue this tradition by aliasing
  # eql? to their overridden #== method, but there are exceptions.
  # [`Numeric`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html) types, for
  # example, perform type conversion across #==, but not across eql?, so:
  #
  # ```ruby
  # 1 == 1.0     #=> true
  # 1.eql? 1.0   #=> false
  # ```
  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(other); end

  # Returns an integer identifier for `obj`.
  #
  # The same number will be returned on all calls to `object_id` for a given
  # object, and no two active objects will share an id.
  #
  # Note: that some objects of builtin classes are reused for optimization. This
  # is the case for immediate values and frozen string literals.
  #
  # [`BasicObject`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html)
  # implements +\_\_id\_\_+,
  # [`Kernel`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html) implements
  # `object_id`.
  #
  # Immediate values are not passed by reference but are passed by value: `nil`,
  # `true`, `false`, Fixnums, Symbols, and some Floats.
  #
  # ```ruby
  # Object.new.object_id  == Object.new.object_id  # => false
  # (21 * 2).object_id    == (21 * 2).object_id    # => true
  # "hello".object_id     == "hello".object_id     # => false
  # "hi".freeze.object_id == "hi".freeze.object_id # => true
  # ```
  sig {returns(Integer)}
  def __id__(); end

  # Invokes the method identified by *symbol*, passing it any arguments
  # specified. You can use `__send__` if the name `send` clashes with an
  # existing method in *obj*. When the method is identified by a string, the
  # string is converted to a symbol.
  #
  # [`BasicObject`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html)
  # implements +\_\_send\_\_+,
  # [`Kernel`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html) implements
  # `send`.
  #
  # ```ruby
  # class Klass
  #   def hello(*args)
  #     "Hello " + args.join(' ')
  #   end
  # end
  # k = Klass.new
  # k.send :hello, "gentle", "readers"   #=> "Hello gentle readers"
  # ```
  sig do
    params(
        arg0: Symbol,
        arg1: BasicObject,
    )
    .returns(T.untyped)
  end
  def __send__(arg0, *arg1); end

  # Equality --- At the
  # [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) level, #==
  # returns `true` only if `obj` and `other` are the same object. Typically,
  # this method is overridden in descendant classes to provide class-specific
  # meaning.
  #
  # Unlike #==, the
  # [`equal?`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html#method-i-equal-3F)
  # method should never be overridden by subclasses as it is used to determine
  # object identity (that is, `a.equal?(b)` if and only if `a` is the same
  # object as `b`):
  #
  # ```ruby
  # obj = "a"
  # other = obj.dup
  #
  # obj == other      #=> true
  # obj.equal? other  #=> false
  # obj.equal? obj    #=> true
  # ```
  #
  # The eql? method returns `true` if `obj` and `other` refer to the same hash
  # key. This is used by [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html)
  # to test members for equality. For any pair of objects where eql? returns
  # `true`, the hash value of both objects must be equal. So any subclass that
  # overrides eql? should also override hash appropriately.
  #
  # For objects of class
  # [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html), eql?  is
  # synonymous with #==. Subclasses normally continue this tradition by aliasing
  # eql? to their overridden #== method, but there are exceptions.
  # [`Numeric`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html) types, for
  # example, perform type conversion across #==, but not across eql?, so:
  #
  # ```ruby
  # 1 == 1.0     #=> true
  # 1.eql? 1.0   #=> false
  # ```
  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def equal?(other); end

  # Default constructor. Does not do anything interesting.
  sig {void}
  def initialize(); end

  # Evaluates a string containing Ruby source code, or the given block, within
  # the context of the receiver (*obj*). In order to set the context, the
  # variable `self` is set to *obj* while the code is executing, giving the code
  # access to *obj*'s instance variables and private methods.
  #
  # When `instance_eval` is given a block, *obj* is also passed in as the
  # block's only argument.
  #
  # When `instance_eval` is given a `String`, the optional second and third
  # parameters supply a filename and starting line number that are used when
  # reporting compilation errors.
  #
  # ```ruby
  # class KlassWithSecret
  #   def initialize
  #     @secret = 99
  #   end
  #   private
  #   def the_secret
  #     "Ssssh! The secret is #{@secret}."
  #   end
  # end
  # k = KlassWithSecret.new
  # k.instance_eval { @secret }          #=> 99
  # k.instance_eval { the_secret }       #=> "Ssssh! The secret is 99."
  # k.instance_eval {|obj| obj == self } #=> true
  # ```
  sig do
    params(
        arg0: String,
        filename: String,
        lineno: Integer,
    )
    .returns(T.untyped)
  end
  sig do
    type_parameters(:U)
    .params(
        blk: T.proc.bind(T.untyped).returns(T.type_parameter(:U)),
    )
    .returns(T.type_parameter(:U))
  end
  def instance_eval(arg0=T.unsafe(nil), filename=T.unsafe(nil), lineno=T.unsafe(nil), &blk); end

  # Executes the given block within the context of the receiver (*obj*). In
  # order to set the context, the variable `self` is set to *obj* while the code
  # is executing, giving the code access to *obj*'s instance variables.
  # Arguments are passed as block parameters.
  #
  # ```ruby
  # class KlassWithSecret
  #   def initialize
  #     @secret = 99
  #   end
  # end
  # k = KlassWithSecret.new
  # k.instance_exec(5) {|x| @secret+x }   #=> 104
  # ```
  sig do
    type_parameters(:U, :V)
    .params(
        args: T.type_parameter(:V),
        blk: T.proc.bind(T.untyped).params(args: T.untyped).returns(T.type_parameter(:U)),
    )
    .returns(T.type_parameter(:U))
  end
  def instance_exec(*args, &blk); end

  sig {overridable.params(method: Symbol).returns(T.untyped)}
  private def singleton_method_added(method); end

  sig {params(method: Symbol, args: T.untyped).returns(T.untyped)}
  private def method_missing(method, *args); end
end
