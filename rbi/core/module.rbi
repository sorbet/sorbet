# typed: __STDLIB_INTERNAL

# A [`Module`](https://docs.ruby-lang.org/en/2.7.0/Module.html) is a collection
# of methods and constants. The methods in a module may be instance methods or
# module methods. Instance methods appear as methods in a class when the module
# is included, module methods do not. Conversely, module methods may be called
# without creating an encapsulating object, while instance methods may not. (See
# [`Module#module_function`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-module_function).)
#
# In the descriptions that follow, the parameter *sym* refers to a symbol, which
# is either a quoted string or a
# [`Symbol`](https://docs.ruby-lang.org/en/2.7.0/Symbol.html) (such as `:name`).
#
# ```ruby
# module Mod
#   include Math
#   CONST = 1
#   def meth
#     #  ...
#   end
# end
# Mod.class              #=> Module
# Mod.constants          #=> [:CONST, :PI, :E]
# Mod.instance_methods   #=> [:meth]
# ```
class Module < Object
  # In the first form, returns an array of the names of all constants accessible
  # from the point of call. This list includes the names of all modules and
  # classes defined in the global scope.
  #
  # ```ruby
  # Module.constants.first(4)
  #    # => [:ARGF, :ARGV, :ArgumentError, :Array]
  #
  # Module.constants.include?(:SEEK_SET)   # => false
  #
  # class IO
  #   Module.constants.include?(:SEEK_SET) # => true
  # end
  # ```
  #
  # The second form calls the instance method `constants`.
  sig {returns(T::Array[Integer])}
  def self.constants(); end

  # Returns the list of `Modules` nested at the point of call.
  #
  # ```ruby
  # module M1
  #   module M2
  #     $a = Module.nesting
  #   end
  # end
  # $a           #=> [M1::M2, M1]
  # $a[0].name   #=> "M1::M2"
  # ```
  sig {returns(T::Array[Module])}
  def self.nesting(); end

  # Returns true if *mod* is a subclass of *other*. Returns `nil` if there's no
  # relationship between the two. (Think of the relationship in terms of the
  # class definition: "class A < B" implies "A < B".)
  sig do
    params(
        other: Module,
    )
    .returns(T.nilable(T::Boolean))
  end
  def <(other); end

  # Returns true if *mod* is a subclass of *other* or is the same as *other*.
  # Returns `nil` if there's no relationship between the two. (Think of the
  # relationship in terms of the class definition: "class A < B" implies "A <
  # B".)
  sig do
    params(
        other: Module,
    )
    .returns(T.nilable(T::Boolean))
  end
  def <=(other); end

  # Comparison---Returns -1, 0, +1 or nil depending on whether `module` includes
  # `other_module`, they are the same, or if `module` is included by
  # `other_module`.
  #
  # Returns `nil` if `module` has no relationship with `other_module`, if
  # `other_module` is not a module, or if the two values are incomparable.
  sig do
    params(
        other: Object,
    )
    .returns(T.nilable(Integer))
  end
  def <=>(other); end

  # Equality --- At the
  # [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html) level,
  # [`==`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-3D-3D)
  # returns `true` only if `obj` and `other` are the same object. Typically,
  # this method is overridden in descendant classes to provide class-specific
  # meaning.
  #
  # Unlike
  # [`==`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-3D-3D), the
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
  # The
  # [`eql?`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-eql-3F)
  # method returns `true` if `obj` and `other` refer to the same hash key. This
  # is used by [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) to test
  # members for equality. For any pair of objects where
  # [`eql?`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-eql-3F)
  # returns `true`, the
  # [`hash`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-hash)
  # value of both objects must be equal. So any subclass that overrides
  # [`eql?`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-eql-3F)
  # should also override
  # [`hash`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-hash)
  # appropriately.
  #
  # For objects of class
  # [`Object`](https://docs.ruby-lang.org/en/2.7.0/Object.html),
  # [`eql?`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-eql-3F)
  # is synonymous with
  # [`==`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-3D-3D).
  # Subclasses normally continue this tradition by aliasing
  # [`eql?`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-eql-3F) to
  # their overridden
  # [`==`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-3D-3D)
  # method, but there are exceptions.
  # [`Numeric`](https://docs.ruby-lang.org/en/2.7.0/Numeric.html) types, for
  # example, perform type conversion across
  # [`==`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-3D-3D), but
  # not across
  # [`eql?`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-eql-3F),
  # so:
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

  # Case Equality---Returns `true` if *obj* is an instance of *mod* or an
  # instance of one of *mod*'s descendants. Of limited use for modules, but can
  # be used in `case` statements to classify objects by class.
  sig do
    type_parameters(:U)
    .params(
        other: T.type_parameter(:U),
    )
    .returns(T::Boolean)
  end
  def ===(other); end

  # Returns true if *mod* is an ancestor of *other*. Returns `nil` if there's no
  # relationship between the two. (Think of the relationship in terms of the
  # class definition: "class A < B" implies "B > A".)
  sig do
    params(
        other: Module,
    )
    .returns(T.nilable(T::Boolean))
  end
  def >(other); end

  # Returns true if *mod* is an ancestor of *other*, or the two modules are the
  # same. Returns `nil` if there's no relationship between the two. (Think of
  # the relationship in terms of the class definition: "class A < B" implies "B
  # > A".)
  sig do
    params(
        other: Module,
    )
    .returns(T.nilable(T::Boolean))
  end
  def >=(other); end

  # Makes *new\_name* a new copy of the method *old\_name*. This can be used to
  # retain access to methods that are overridden.
  #
  # ```ruby
  # module Mod
  #   alias_method :orig_exit, :exit #=> :orig_exit
  #   def exit(code=0)
  #     puts "Exiting with code #{code}"
  #     orig_exit(code)
  #   end
  # end
  # include Mod
  # exit(99)
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # Exiting with code 99
  # ```
  sig do
    params(
        new_name: Symbol,
        old_name: Symbol,
    )
    .returns(Symbol)
  end
  def alias_method(new_name, old_name); end

  # Returns a list of modules included/prepended in *mod* (including *mod*
  # itself).
  #
  # ```ruby
  # module Mod
  #   include Math
  #   include Comparable
  #   prepend Enumerable
  # end
  #
  # Mod.ancestors        #=> [Enumerable, Mod, Comparable, Math]
  # Math.ancestors       #=> [Math]
  # Enumerable.ancestors #=> [Enumerable]
  # ```
  sig {returns(T::Array[Module])}
  def ancestors(); end

  # When this module is included in another, Ruby calls
  # [`append_features`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-append_features)
  # in this module, passing it the receiving module in *mod*. Ruby's default
  # implementation is to add the constants, methods, and module variables of
  # this module to *mod* if this module has not already been added to *mod* or
  # one of its ancestors. See also
  # [`Module#include`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-include).
  sig do
    params(
        arg0: Module,
    )
    .returns(T.self_type)
  end
  def append_features(arg0); end

  # Defines a named attribute for this module, where the name is
  # *symbol.*`id2name`, creating an instance variable (`@name`) and a
  # corresponding access method to read it. Also creates a method called `name=`
  # to set the attribute.
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) arguments are
  # converted to symbols. Returns an array of defined method names as symbols.
  #
  # ```ruby
  # module Mod
  #   attr_accessor(:one, :two) #=> [:one, :one=, :two, :two=]
  # end
  # Mod.instance_methods.sort   #=> [:one, :one=, :two, :two=]
  # ```
  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(NilClass)
  end
  def attr_accessor(*arg0); end

  # Creates instance variables and corresponding methods that return the value
  # of each instance variable. Equivalent to calling "`attr`*:name*" on each
  # name in turn. [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # arguments are converted to symbols. Returns an array of defined method names
  # as symbols.
  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(NilClass)
  end
  def attr_reader(*arg0); end

  # Creates an accessor method to allow assignment to the attribute
  # *symbol*`.id2name`.
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) arguments are
  # converted to symbols. Returns an array of defined method names as symbols.
  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(NilClass)
  end
  def attr_writer(*arg0); end

  # Registers *filename* to be loaded (using Kernel::require) the first time
  # that *module* (which may be a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) or a symbol) is
  # accessed in the namespace of *mod*.
  #
  # ```ruby
  # module A
  # end
  # A.autoload(:B, "b")
  # A::B.doit            # autoloads "b"
  # ```
  sig do
    params(
        _module: Symbol,
        filename: String,
    )
    .returns(NilClass)
  end
  def autoload(_module, filename); end

  # Returns *filename* to be loaded if *name* is registered as `autoload` in the
  # namespace of *mod* or one of its ancestors.
  #
  # ```ruby
  # module A
  # end
  # A.autoload(:B, "b")
  # A.autoload?(:B)            #=> "b"
  # ```
  #
  # If `inherit` is false, the lookup only checks the autoloads in the receiver:
  #
  # ```ruby
  # class A
  #   autoload :CONST, "const.rb"
  # end
  #
  # class B < A
  # end
  #
  # B.autoload?(:CONST)          #=> "const.rb", found in A (ancestor)
  # B.autoload?(:CONST, false)   #=> nil, not found in B itself
  # ```
  sig do
    params(
        name: T.any(Symbol, String),
        inherit: T.nilable(T::Boolean),
    )
    .returns(T.nilable(String))
  end
  def autoload?(name, inherit = true); end

  # Evaluates the string or block in the context of *mod*, except that when a
  # block is given, constant/class variable lookup is not affected. This can be
  # used to add methods to a class. `module_eval` returns the result of
  # evaluating its argument. The optional *filename* and *lineno* parameters set
  # the text for error messages.
  #
  # ```ruby
  # class Thing
  # end
  # a = %q{def hello() "Hello there!" end}
  # Thing.module_eval(a)
  # puts Thing.new.hello()
  # Thing.module_eval("invalid code", "dummy", 123)
  # ```
  #
  # *produces:*
  #
  # ```
  # Hello there!
  # dummy:123:in `module_eval': undefined local variable
  #     or method `code' for Thing:Class
  # ```
  #
  #
  # Alias for:
  # [`module_eval`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-module_eval)
  sig do
    params(
        arg0: String,
        filename: String,
        lineno: Integer,
    )
    .returns(T.untyped)
  end
  sig do
    type_parameters(:U).params(
        blk: T.proc
              .bind(T.untyped)
              .params(m: T.untyped)
              .returns(T.type_parameter(:U))
    )
    .returns(T.type_parameter(:U))
  end
  def class_eval(arg0, filename=T.unsafe(nil), lineno=T.unsafe(nil), &blk); end

  # Evaluates the given block in the context of the class/module. The method
  # defined in the block will belong to the receiver. Any arguments passed to
  # the method will be passed to the block. This can be used if the block needs
  # to access instance variables.
  #
  # ```ruby
  # class Thing
  # end
  # Thing.class_exec{
  #   def hello() "Hello there!" end
  # }
  # puts Thing.new.hello()
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # Hello there!
  # ```
  #
  #
  # Alias for:
  # [`module_exec`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-module_exec)
  sig do
    params(
        args: BasicObject,
        blk: T.untyped,
    )
    .returns(T.untyped)
  end
  def class_exec(*args, &blk); end

  # Returns `true` if the given class variable is defined in *obj*.
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) arguments are
  # converted to symbols.
  #
  # ```ruby
  # class Fred
  #   @@foo = 99
  # end
  # Fred.class_variable_defined?(:@@foo)    #=> true
  # Fred.class_variable_defined?(:@@bar)    #=> false
  # ```
  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T::Boolean)
  end
  def class_variable_defined?(arg0); end

  # Returns the value of the given class variable (or throws a
  # [`NameError`](https://docs.ruby-lang.org/en/2.7.0/NameError.html)
  # exception). The `@@` part of the variable name should be included for
  # regular class variables.
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) arguments are
  # converted to symbols.
  #
  # ```ruby
  # class Fred
  #   @@foo = 99
  # end
  # Fred.class_variable_get(:@@foo)     #=> 99
  # ```
  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.untyped)
  end
  def class_variable_get(arg0); end

  # Sets the class variable named by *symbol* to the given object. If the class
  # variable name is passed as a string, that string is converted to a symbol.
  #
  # ```ruby
  # class Fred
  #   @@foo = 99
  #   def foo
  #     @@foo
  #   end
  # end
  # Fred.class_variable_set(:@@foo, 101)     #=> 101
  # Fred.new.foo                             #=> 101
  # ```
  sig do
    params(
        arg0: T.any(Symbol, String),
        arg1: BasicObject,
    )
    .returns(T.untyped)
  end
  def class_variable_set(arg0, arg1); end

  # Returns an array of the names of class variables in *mod*. This includes the
  # names of class variables in any included modules, unless the *inherit*
  # parameter is set to `false`.
  #
  # ```ruby
  # class One
  #   @@var1 = 1
  # end
  # class Two < One
  #   @@var2 = 2
  # end
  # One.class_variables          #=> [:@@var1]
  # Two.class_variables          #=> [:@@var2, :@@var1]
  # Two.class_variables(false)   #=> [:@@var2]
  # ```
  sig do
    params(
        inherit: T::Boolean,
    )
    .returns(T::Array[Symbol])
  end
  def class_variables(inherit=T.unsafe(nil)); end

  # Says whether *mod* or its ancestors have a constant with the given name:
  #
  # ```ruby
  # Float.const_defined?(:EPSILON)      #=> true, found in Float itself
  # Float.const_defined?("String")      #=> true, found in Object (ancestor)
  # BasicObject.const_defined?(:Hash)   #=> false
  # ```
  #
  # If *mod* is a `Module`, additionally `Object` and its ancestors are checked:
  #
  # ```ruby
  # Math.const_defined?(:String)   #=> true, found in Object
  # ```
  #
  # In each of the checked classes or modules, if the constant is not present
  # but there is an autoload for it, `true` is returned directly without
  # autoloading:
  #
  # ```ruby
  # module Admin
  #   autoload :User, 'admin/user'
  # end
  # Admin.const_defined?(:User)   #=> true
  # ```
  #
  # If the constant is not found the callback `const_missing` is **not** called
  # and the method returns `false`.
  #
  # If `inherit` is false, the lookup only checks the constants in the receiver:
  #
  # ```ruby
  # IO.const_defined?(:SYNC)          #=> true, found in File::Constants (ancestor)
  # IO.const_defined?(:SYNC, false)   #=> false, not found in IO itself
  # ```
  #
  # In this case, the same logic for autoloading applies.
  #
  # If the argument is not a valid constant name a `NameError` is raised with
  # the message "wrong constant name *name*":
  #
  # ```ruby
  # Hash.const_defined? 'foobar'   #=> NameError: wrong constant name foobar
  # ```
  sig do
    params(
        arg0: T.any(Symbol, String),
        inherit: T::Boolean,
    )
    .returns(T::Boolean)
  end
  def const_defined?(arg0, inherit=T.unsafe(nil)); end

  # Checks for a constant with the given name in *mod*. If `inherit` is set, the
  # lookup will also search the ancestors (and `Object` if *mod* is a `Module`).
  #
  # The value of the constant is returned if a definition is found, otherwise a
  # `NameError` is raised.
  #
  # ```ruby
  # Math.const_get(:PI)   #=> 3.14159265358979
  # ```
  #
  # This method will recursively look up constant names if a namespaced class
  # name is provided. For example:
  #
  # ```ruby
  # module Foo; class Bar; end end
  # Object.const_get 'Foo::Bar'
  # ```
  #
  # The `inherit` flag is respected on each lookup. For example:
  #
  # ```ruby
  # module Foo
  #   class Bar
  #     VAL = 10
  #   end
  #
  #   class Baz < Bar; end
  # end
  #
  # Object.const_get 'Foo::Baz::VAL'         # => 10
  # Object.const_get 'Foo::Baz::VAL', false  # => NameError
  # ```
  #
  # If the argument is not a valid constant name a `NameError` will be raised
  # with a warning "wrong constant name".
  #
  # ```ruby
  # Object.const_get 'foobar' #=> NameError: wrong constant name foobar
  # ```
  sig do
    params(
        arg0: T.any(Symbol, String),
        inherit: T::Boolean,
    )
    .returns(T.untyped)
  end
  def const_get(arg0, inherit=T.unsafe(nil)); end

  # Invoked when a reference is made to an undefined constant in *mod*. It is
  # passed a symbol for the undefined constant, and returns a value to be used
  # for that constant. The following code is an example of the same:
  #
  # ```ruby
  # def Foo.const_missing(name)
  #   name # return the constant name as Symbol
  # end
  #
  # Foo::UNDEFINED_CONST    #=> :UNDEFINED_CONST: symbol returned
  # ```
  #
  # In the next example when a reference is made to an undefined constant, it
  # attempts to load a file whose name is the lowercase version of the constant
  # (thus class `Fred` is assumed to be in file `fred.rb`). If found, it returns
  # the loaded class. It therefore implements an autoload feature similar to
  # [`Kernel#autoload`](https://docs.ruby-lang.org/en/2.7.0/Kernel.html#method-i-autoload)
  # and
  # [`Module#autoload`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-autoload).
  #
  # ```ruby
  # def Object.const_missing(name)
  #   @looked_for ||= {}
  #   str_name = name.to_s
  #   raise "Class not found: #{name}" if @looked_for[str_name]
  #   @looked_for[str_name] = 1
  #   file = str_name.downcase
  #   require file
  #   klass = const_get(name)
  #   return klass if klass
  #   raise "Class not found: #{name}"
  # end
  # ```
  sig do
    params(
        arg0: Symbol,
    )
    .returns(T.untyped)
  end
  def const_missing(arg0); end

  # Sets the named constant to the given object, returning that object. Creates
  # a new constant if no constant with the given name previously existed.
  #
  # ```ruby
  # Math.const_set("HIGH_SCHOOL_PI", 22.0/7.0)   #=> 3.14285714285714
  # Math::HIGH_SCHOOL_PI - Math::PI              #=> 0.00126448926734968
  # ```
  #
  # If `sym` or `str` is not a valid constant name a `NameError` will be raised
  # with a warning "wrong constant name".
  #
  # ```ruby
  # Object.const_set('foobar', 42) #=> NameError: wrong constant name foobar
  # ```
  sig do
    params(
        arg0: T.any(Symbol, String),
        arg1: BasicObject,
    )
    .returns(T.untyped)
  end
  def const_set(arg0, arg1); end

  # Returns the Ruby source filename and line number containing the definition
  # of the constant specified. If the named constant is not found, `nil` is
  # returned. If the constant is found, but its source location can not be
  # extracted (constant is defined in C code), empty array is returned.
  #
  # *inherit* specifies whether to lookup in `mod.ancestors` (`true` by
  # default).
  #
  # ```ruby
  # # test.rb:
  # class A         # line 1
  #   C1 = 1
  #   C2 = 2
  # end
  #
  # module M        # line 6
  #   C3 = 3
  # end
  #
  # class B < A     # line 10
  #   include M
  #   C4 = 4
  # end
  #
  # class A # continuation of A definition
  #   C2 = 8 # constant redefinition; warned yet allowed
  # end
  #
  # p B.const_source_location('C4')           # => ["test.rb", 12]
  # p B.const_source_location('C3')           # => ["test.rb", 7]
  # p B.const_source_location('C1')           # => ["test.rb", 2]
  #
  # p B.const_source_location('C3', false)    # => nil  -- don't lookup in ancestors
  #
  # p A.const_source_location('C2')           # => ["test.rb", 16] -- actual (last) definition place
  #
  # p Object.const_source_location('B')       # => ["test.rb", 10] -- top-level constant could be looked through Object
  # p Object.const_source_location('A')       # => ["test.rb", 1] -- class reopening is NOT considered new definition
  #
  # p B.const_source_location('A')            # => ["test.rb", 1]  -- because Object is in ancestors
  # p M.const_source_location('A')            # => ["test.rb", 1]  -- Object is not ancestor, but additionally checked for modules
  #
  # p Object.const_source_location('A::C1')   # => ["test.rb", 2]  -- nesting is supported
  # p Object.const_source_location('String')  # => []  -- constant is defined in C code
  # ```
  sig do
    params(
        sym: T.any(Symbol, String),
        inherit: T::Boolean,
    )
    .returns(T.nilable([String, Integer]))
  end
  def const_source_location(sym, inherit=true); end

  # Returns an array of the names of the constants accessible in *mod*. This
  # includes the names of constants in any included modules (example at start of
  # section), unless the *inherit* parameter is set to `false`.
  #
  # The implementation makes no guarantees about the order in which the
  # constants are yielded.
  #
  # ```ruby
  # IO.constants.include?(:SYNC)        #=> true
  # IO.constants(false).include?(:SYNC) #=> false
  # ```
  #
  # Also see
  # [`Module#const_defined?`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-const_defined-3F).
  sig do
    params(
        inherit: T::Boolean,
    )
    .returns(T::Array[Symbol])
  end
  def constants(inherit=T.unsafe(nil)); end

  # Defines an instance method in the receiver. The *method* parameter can be a
  # `Proc`, a `Method` or an `UnboundMethod` object. If a block is specified, it
  # is used as the method body. If a block or the *method* parameter has
  # parameters, they're used as method parameters. This block is evaluated using
  # [`instance_eval`](https://docs.ruby-lang.org/en/2.7.0/BasicObject.html#method-i-instance_eval).
  #
  # ```ruby
  # class A
  #   def fred
  #     puts "In Fred"
  #   end
  #   def create_method(name, &block)
  #     self.class.define_method(name, &block)
  #   end
  #   define_method(:wilma) { puts "Charge it!" }
  #   define_method(:flint) {|name| puts "I'm #{name}!"}
  # end
  # class B < A
  #   define_method(:barney, instance_method(:fred))
  # end
  # a = B.new
  # a.barney
  # a.wilma
  # a.flint('Dino')
  # a.create_method(:betty) { p self }
  # a.betty
  # ```
  #
  # *produces:*
  #
  # ```
  # In Fred
  # Charge it!
  # I'm Dino!
  # #<B:0x401b39e8>
  # ```
  sig do
    params(
        arg0: T.any(Symbol, String),
        arg1: T.any(Proc, Method, UnboundMethod)
    )
    .returns(Symbol)
  end
  sig do
    params(
        arg0: T.any(Symbol, String),
        blk: T.untyped,
    )
    .returns(Symbol)
  end
  def define_method(arg0, arg1=T.unsafe(nil), &blk); end

  # Makes a list of existing constants deprecated. Attempt to refer to them will
  # produce a warning.
  #
  # ```ruby
  # module HTTP
  #   NotFound = Exception.new
  #   NOT_FOUND = NotFound # previous version of the library used this name
  #
  #   deprecate_constant :NOT_FOUND
  # end
  #
  # HTTP::NOT_FOUND
  # # warning: constant HTTP::NOT_FOUND is deprecated
  # ```
  def deprecate_constant(*_); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def eql?(other); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def equal?(other); end

  # Extends the specified object by adding this module's constants and methods
  # (which are added as singleton methods). This is the callback method used by
  # [`Object#extend`](https://docs.ruby-lang.org/en/2.7.0/Object.html#method-i-extend).
  #
  # ```ruby
  # module Picky
  #   def Picky.extend_object(o)
  #     if String === o
  #       puts "Can't add Picky to a String"
  #     else
  #       puts "Picky added to #{o.class}"
  #       super
  #     end
  #   end
  # end
  # (s = Array.new).extend Picky  # Call Object.extend
  # (s = "quick brown fox").extend Picky
  # ```
  #
  # *produces:*
  #
  # ```
  # Picky added to Array
  # Can't add Picky to a String
  # ```
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T.untyped)
  end
  def extend_object(arg0); end

  # The equivalent of `included`, but for extended modules.
  #
  # ```ruby
  # module A
  #   def self.extended(mod)
  #     puts "#{self} extended in #{mod}"
  #   end
  # end
  # module Enumerable
  #   extend A
  # end
  #  # => prints "A extended in Enumerable"
  # ```
  sig do
    params(
        othermod: Module,
    )
    .returns(T.untyped)
  end
  def extended(othermod); end

  # Prevents further modifications to *mod*.
  #
  # This method returns self.
  sig {returns(T.self_type)}
  def freeze(); end

  # Invokes
  # [`Module.append_features`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-append_features)
  # on each parameter in reverse order.
  sig do
    params(
        arg0: Module,
    )
    .returns(T.self_type)
  end
  def include(*arg0); end

  # Returns `true` if *module* is included or prepended in *mod* or one of
  # *mod*'s ancestors.
  #
  # ```ruby
  # module A
  # end
  # class B
  #   include A
  # end
  # class C < B
  # end
  # B.include?(A)   #=> true
  # C.include?(A)   #=> true
  # A.include?(A)   #=> false
  # ```
  sig do
    params(
        arg0: Module,
    )
    .returns(T::Boolean)
  end
  def include?(arg0); end

  # Callback invoked whenever the receiver is included in another module or
  # class. This should be used in preference to `Module.append_features` if your
  # code wants to perform some action when a module is included in another.
  #
  # ```ruby
  # module A
  #   def A.included(mod)
  #     puts "#{self} included in #{mod}"
  #   end
  # end
  # module Enumerable
  #   include A
  # end
  #  # => prints "A included in Enumerable"
  # ```
  sig do
    params(
        othermod: Module,
    )
    .returns(T.untyped)
  end
  def included(othermod); end

  # Returns the list of modules included or prepended in *mod* or one of *mod*'s
  # ancestors.
  #
  # ```ruby
  # module Sub
  # end
  #
  # module Mixin
  #   prepend Sub
  # end
  #
  # module Outer
  #   include Mixin
  # end
  #
  # Mixin.included_modules   #=> [Sub]
  # Outer.included_modules   #=> [Sub, Mixin]
  # ```
  sig {returns(T::Array[Module])}
  def included_modules(); end

  sig {returns(Object)}
  sig do
    params(
        blk: T.proc.params(arg0: Module).returns(BasicObject),
    )
    .void
  end
  def initialize(&blk); end

  # Returns an `UnboundMethod` representing the given instance method in *mod*.
  #
  # ```ruby
  # class Interpreter
  #   def do_a() print "there, "; end
  #   def do_d() print "Hello ";  end
  #   def do_e() print "!\n";     end
  #   def do_v() print "Dave";    end
  #   Dispatcher = {
  #     "a" => instance_method(:do_a),
  #     "d" => instance_method(:do_d),
  #     "e" => instance_method(:do_e),
  #     "v" => instance_method(:do_v)
  #   }
  #   def interpret(string)
  #     string.each_char {|b| Dispatcher[b].bind(self).call }
  #   end
  # end
  #
  # interpreter = Interpreter.new
  # interpreter.interpret('dave')
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # Hello there, Dave!
  # ```
  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(UnboundMethod)
  end
  def instance_method(arg0); end

  # Returns an array containing the names of the public and protected instance
  # methods in the receiver. For a module, these are the public and protected
  # methods; for a class, they are the instance (not singleton) methods. If the
  # optional parameter is `false`, the methods of any ancestors are not
  # included.
  #
  # ```ruby
  # module A
  #   def method1()  end
  # end
  # class B
  #   include A
  #   def method2()  end
  # end
  # class C < B
  #   def method3()  end
  # end
  #
  # A.instance_methods(false)                   #=> [:method1]
  # B.instance_methods(false)                   #=> [:method2]
  # B.instance_methods(true).include?(:method1) #=> true
  # C.instance_methods(false)                   #=> [:method3]
  # C.instance_methods.include?(:method2)       #=> true
  # ```
  sig do
    params(
        include_super: T::Boolean,
    )
    .returns(T::Array[Symbol])
  end
  def instance_methods(include_super=T.unsafe(nil)); end

  # Invoked as a callback whenever an instance method is added to the receiver.
  #
  # ```ruby
  # module Chatty
  #   def self.method_added(method_name)
  #     puts "Adding #{method_name.inspect}"
  #   end
  #   def self.some_class_method() end
  #   def some_instance_method() end
  # end
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # Adding :some_instance_method
  # ```
  sig do
    params(
        meth: Symbol,
    )
    .returns(T.untyped)
  end
  def method_added(meth); end

  # Returns `true` if the named method is defined by *mod*. If *inherit* is set,
  # the lookup will also search *mod*'s ancestors. Public and protected methods
  # are matched. [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # arguments are converted to symbols.
  #
  # ```ruby
  # module A
  #   def method1()  end
  #   def protected_method1()  end
  #   protected :protected_method1
  # end
  # class B
  #   def method2()  end
  #   def private_method2()  end
  #   private :private_method2
  # end
  # class C < B
  #   include A
  #   def method3()  end
  # end
  #
  # A.method_defined? :method1              #=> true
  # C.method_defined? "method1"             #=> true
  # C.method_defined? "method2"             #=> true
  # C.method_defined? "method2", true       #=> true
  # C.method_defined? "method2", false      #=> false
  # C.method_defined? "method3"             #=> true
  # C.method_defined? "protected_method1"   #=> true
  # C.method_defined? "method4"             #=> false
  # C.method_defined? "private_method2"     #=> false
  # ```
  sig do
    params(
        method_name: T.any(Symbol, String),
        inherit: T::Boolean,
    )
    .returns(T::Boolean)
  end
  def method_defined?(method_name, inherit=true); end

  # Invoked as a callback whenever an instance method is removed from the
  # receiver.
  #
  # ```ruby
  # module Chatty
  #   def self.method_removed(method_name)
  #     puts "Removing #{method_name.inspect}"
  #   end
  #   def self.some_class_method() end
  #   def some_instance_method() end
  #   class << self
  #     remove_method :some_class_method
  #   end
  #   remove_method :some_instance_method
  # end
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # Removing :some_instance_method
  # ```
  sig do
    params(
        method_name: Symbol,
    )
    .returns(T.untyped)
  end
  def method_removed(method_name); end

  # Evaluates the string or block in the context of *mod*, except that when a
  # block is given, constant/class variable lookup is not affected. This can be
  # used to add methods to a class. `module_eval` returns the result of
  # evaluating its argument. The optional *filename* and *lineno* parameters set
  # the text for error messages.
  #
  # ```ruby
  # class Thing
  # end
  # a = %q{def hello() "Hello there!" end}
  # Thing.module_eval(a)
  # puts Thing.new.hello()
  # Thing.module_eval("invalid code", "dummy", 123)
  # ```
  #
  # *produces:*
  #
  # ```
  # Hello there!
  # dummy:123:in `module_eval': undefined local variable
  #     or method `code' for Thing:Class
  # ```
  #
  #
  # Also aliased as:
  # [`class_eval`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-class_eval)
  sig do
    params(
        arg0: String,
        filename: String,
        lineno: Integer,
    )
    .returns(T.untyped)
  end
  sig do
    type_parameters(:U).params(
        blk: T.proc
              .bind(T.untyped)
              .params(m: T.untyped)
              .returns(T.type_parameter(:U))
    )
    .returns(T.type_parameter(:U))
  end
  def module_eval(arg0, filename=T.unsafe(nil), lineno=T.unsafe(nil), &blk); end

  # Evaluates the given block in the context of the class/module. The method
  # defined in the block will belong to the receiver. Any arguments passed to
  # the method will be passed to the block. This can be used if the block needs
  # to access instance variables.
  #
  # ```ruby
  # class Thing
  # end
  # Thing.class_exec{
  #   def hello() "Hello there!" end
  # }
  # puts Thing.new.hello()
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # Hello there!
  # ```
  #
  #
  # Also aliased as:
  # [`class_exec`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-class_exec)
  sig do
    params(
        args: BasicObject,
        blk: T.untyped,
    )
    .returns(T.untyped)
  end
  def module_exec(*args, &blk); end

  # Creates module functions for the named methods. These functions may be
  # called with the module as a receiver, and also become available as instance
  # methods to classes that mix in the module.
  # [`Module`](https://docs.ruby-lang.org/en/2.7.0/Module.html) functions are
  # copies of the original, and so may be changed independently. The
  # instance-method versions are made private. If used with no arguments,
  # subsequently defined methods become module functions.
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) arguments are
  # converted to symbols. If a single argument is passed, it is returned. If no
  # argument is passed, nil is returned. If multiple arguments are passed, the
  # arguments are returned as an array.
  #
  # ```ruby
  # module Mod
  #   def one
  #     "This is one"
  #   end
  #   module_function :one
  # end
  # class Cls
  #   include Mod
  #   def call_one
  #     one
  #   end
  # end
  # Mod.one     #=> "This is one"
  # c = Cls.new
  # c.call_one  #=> "This is one"
  # module Mod
  #   def one
  #     "This is the new one"
  #   end
  # end
  # Mod.one     #=> "This is one"
  # c.call_one  #=> "This is the new one"
  # ```
  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.self_type)
  end
  def module_function(*arg0); end

  # Returns the name of the module *mod*. Returns nil for anonymous modules.
  sig {returns(T.nilable(String))}
  def name(); end

  # Invokes
  # [`Module.prepend_features`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-prepend_features)
  # on each parameter in reverse order.
  sig do
    params(
        arg0: Module,
    )
    .returns(T.self_type)
  end
  def prepend(*arg0); end

  # When this module is prepended in another, Ruby calls
  # [`prepend_features`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-prepend_features)
  # in this module, passing it the receiving module in *mod*. Ruby's default
  # implementation is to overlay the constants, methods, and module variables of
  # this module to *mod* if this module has not already been added to *mod* or
  # one of its ancestors. See also
  # [`Module#prepend`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-prepend).
  sig do
    params(
        arg0: Module,
    )
    .returns(T.self_type)
  end
  def prepend_features(arg0); end

  # The equivalent of `included`, but for prepended modules.
  #
  # ```ruby
  # module A
  #   def self.prepended(mod)
  #     puts "#{self} prepended to #{mod}"
  #   end
  # end
  # module Enumerable
  #   prepend A
  # end
  #  # => prints "A prepended to Enumerable"
  # ```
  sig do
    params(
        othermod: Module,
    )
    .returns(T.untyped)
  end
  def prepended(othermod); end

  # With no arguments, sets the default visibility for subsequently defined
  # methods to private. With arguments, sets the named methods to have private
  # visibility. [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # arguments are converted to symbols. An
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of Symbols and/or
  # Strings is also accepted. If a single argument is passed, it is returned. If
  # no argument is passed, nil is returned. If multiple arguments are passed,
  # the arguments are returned as an array.
  #
  # ```ruby
  # module Mod
  #   def a()  end
  #   def b()  end
  #   private
  #   def c()  end
  #   private :a
  # end
  # Mod.private_instance_methods   #=> [:a, :c]
  # ```
  #
  # Note that to show a private method on
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html), use `:doc:`.
  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.self_type)
  end
  def private(*arg0); end

  # Makes existing class methods private. Often used to hide the default
  # constructor `new`.
  #
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) arguments are
  # converted to symbols. An
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of Symbols and/or
  # Strings is also accepted.
  #
  # ```ruby
  # class SimpleSingleton  # Not thread safe
  #   private_class_method :new
  #   def SimpleSingleton.create(*args, &block)
  #     @me = new(*args, &block) if ! @me
  #     @me
  #   end
  # end
  # ```
  sig do
    params(
        arg0: T.any(T::Array[Symbol], T::Array[String], Symbol, String),
    )
    .returns(T.self_type)
  end
  def private_class_method(*arg0); end

  # Makes a list of existing constants private.
  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.self_type)
  end
  def private_constant(*arg0); end

  # Returns a list of the private instance methods defined in *mod*. If the
  # optional parameter is `false`, the methods of any ancestors are not
  # included.
  #
  # ```ruby
  # module Mod
  #   def method1()  end
  #   private :method1
  #   def method2()  end
  # end
  # Mod.instance_methods           #=> [:method2]
  # Mod.private_instance_methods   #=> [:method1]
  # ```
  sig do
    params(
        include_super: T::Boolean,
    )
    .returns(T::Array[Symbol])
  end
  def private_instance_methods(include_super=T.unsafe(nil)); end

  # Returns `true` if the named private method is defined by *mod*. If *inherit*
  # is set, the lookup will also search *mod*'s ancestors.
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) arguments are
  # converted to symbols.
  #
  # ```ruby
  # module A
  #   def method1()  end
  # end
  # class B
  #   private
  #   def method2()  end
  # end
  # class C < B
  #   include A
  #   def method3()  end
  # end
  #
  # A.method_defined? :method1                   #=> true
  # C.private_method_defined? "method1"          #=> false
  # C.private_method_defined? "method2"          #=> true
  # C.private_method_defined? "method2", true    #=> true
  # C.private_method_defined? "method2", false   #=> false
  # C.method_defined? "method2"                  #=> false
  # ```
  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T::Boolean)
  end
  def private_method_defined?(arg0); end

  # With no arguments, sets the default visibility for subsequently defined
  # methods to protected. With arguments, sets the named methods to have
  # protected visibility.
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) arguments are
  # converted to symbols. An
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of Symbols and/or
  # Strings is also accepted. If a single argument is passed, it is returned. If
  # no argument is passed, nil is returned. If multiple arguments are passed,
  # the arguments are returned as an array.
  #
  # If a method has protected visibility, it is callable only where `self` of
  # the context is the same as the method. (method definition or
  # instance\_eval). This behavior is different from Java's protected method.
  # Usually `private` should be used.
  #
  # Note that a protected method is slow because it can't use inline cache.
  #
  # To show a private method on
  # [`RDoc`](https://docs.ruby-lang.org/en/2.7.0/RDoc.html), use `:doc:` instead
  # of this.
  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.self_type)
  end
  def protected(*arg0); end

  # Returns a list of the protected instance methods defined in *mod*. If the
  # optional parameter is `false`, the methods of any ancestors are not
  # included.
  sig do
    params(
        include_super: T::Boolean,
    )
    .returns(T::Array[Symbol])
  end
  def protected_instance_methods(include_super=T.unsafe(nil)); end

  # Returns `true` if the named protected method is defined *mod*. If *inherit*
  # is set, the lookup will also search *mod*'s ancestors.
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) arguments are
  # converted to symbols.
  #
  # ```ruby
  # module A
  #   def method1()  end
  # end
  # class B
  #   protected
  #   def method2()  end
  # end
  # class C < B
  #   include A
  #   def method3()  end
  # end
  #
  # A.method_defined? :method1                    #=> true
  # C.protected_method_defined? "method1"         #=> false
  # C.protected_method_defined? "method2"         #=> true
  # C.protected_method_defined? "method2", true   #=> true
  # C.protected_method_defined? "method2", false  #=> false
  # C.method_defined? "method2"                   #=> true
  # ```
  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T::Boolean)
  end
  def protected_method_defined?(arg0); end

  # With no arguments, sets the default visibility for subsequently defined
  # methods to public. With arguments, sets the named methods to have public
  # visibility. [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # arguments are converted to symbols. An
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of Symbols and/or
  # Strings is also accepted. If a single argument is passed, it is returned. If
  # no argument is passed, nil is returned. If multiple arguments are passed,
  # the arguments are returned as an array.
  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.self_type)
  end
  def public(*arg0); end

  # Makes a list of existing class methods public.
  #
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) arguments are
  # converted to symbols. An
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of Symbols and/or
  # Strings is also accepted.
  sig do
    params(
        arg0: T.any(T::Array[Symbol], T::Array[String], Symbol, String),
    )
    .returns(T.self_type)
  end
  def public_class_method(*arg0); end

  # Makes a list of existing constants public.
  sig do
    params(
        arg0: Symbol,
    )
    .returns(T.self_type)
  end
  def public_constant(*arg0); end

  # Similar to *instance\_method*, searches public method only.
  sig do
    params(
        arg0: Symbol,
    )
    .returns(UnboundMethod)
  end
  def public_instance_method(arg0); end

  # Returns a list of the public instance methods defined in *mod*. If the
  # optional parameter is `false`, the methods of any ancestors are not
  # included.
  sig do
    params(
        include_super: T::Boolean,
    )
    .returns(T::Array[Symbol])
  end
  def public_instance_methods(include_super=T.unsafe(nil)); end

  # Returns `true` if the named public method is defined by *mod*. If *inherit*
  # is set, the lookup will also search *mod*'s ancestors.
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) arguments are
  # converted to symbols.
  #
  # ```ruby
  # module A
  #   def method1()  end
  # end
  # class B
  #   protected
  #   def method2()  end
  # end
  # class C < B
  #   include A
  #   def method3()  end
  # end
  #
  # A.method_defined? :method1                 #=> true
  # C.public_method_defined? "method1"         #=> true
  # C.public_method_defined? "method1", true   #=> true
  # C.public_method_defined? "method1", false  #=> true
  # C.public_method_defined? "method2"         #=> false
  # C.method_defined? "method2"                #=> true
  # ```
  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T::Boolean)
  end
  def public_method_defined?(arg0); end

  # Refine *mod* in the receiver.
  #
  # Returns a module, where refined methods are defined.
  sig do
    params(
        arg0: Class,
        blk: T.proc.params(arg0: T.untyped).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  def refine(arg0, &blk); end

  # Removes the named class variable from the receiver, returning that
  # variable's value.
  #
  # ```ruby
  # class Example
  #   @@var = 99
  #   puts remove_class_variable(:@@var)
  #   p(defined? @@var)
  # end
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # 99
  # nil
  # ```
  sig do
    params(
        arg0: Symbol,
    )
    .returns(T.untyped)
  end
  def remove_class_variable(arg0); end

  # Removes the definition of the given constant, returning that constant's
  # previous value. If that constant referred to a module, this will not change
  # that module's name and can lead to confusion.
  sig do
    params(
        arg0: Symbol,
    )
    .returns(T.untyped)
  end
  def remove_const(arg0); end

  # Removes the method identified by *symbol* from the current class. For an
  # example, see
  # [`Module#undef_method`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-undef_method).
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) arguments are
  # converted to symbols.
  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.self_type)
  end
  def remove_method(arg0); end

  # For the given method names, marks the method as passing keywords through a
  # normal argument splat. This should only be called on methods that accept an
  # argument splat (`*args`) but not explicit keywords or a keyword splat. It
  # marks the method such that if the method is called with keyword arguments,
  # the final hash argument is marked with a special flag such that if it is the
  # final element of a normal argument splat to another method call, and that
  # method call does not include explicit keywords or a keyword splat, the final
  # element is interpreted as keywords. In other words, keywords will be passed
  # through the method to other methods.
  #
  # This should only be used for methods that delegate keywords to another
  # method, and only for backwards compatibility with Ruby versions before 3.0.
  # See
  # https://www.ruby-lang.org/en/news/2019/12/12/separation-of-positional-and-keyword-arguments-in-ruby-3-0/
  # for details on why `ruby2_keywords` exists and when and how to use it.
  #
  # This method will probably be removed at some point, as it exists only for
  # backwards compatibility. As it does not exist in Ruby versions before 2.7,
  # check that the module responds to this method before calling it:
  #
  # ```ruby
  # module Mod
  #   def foo(meth, *args, &block)
  #     send(:"do_#{meth}", *args, &block)
  #   end
  #   ruby2_keywords(:foo) if respond_to?(:ruby2_keywords, true)
  # end
  # ```
  #
  # However, be aware that if the `ruby2_keywords` method is removed, the
  # behavior of the `foo` method using the above approach will change so that
  # the method does not pass through keywords.
  sig { params(method_name: Symbol).returns(T.self_type) }
  def ruby2_keywords(*method_name); end

  # Returns `true` if *mod* is a singleton class or `false` if it is an ordinary
  # class or module.
  #
  # ```ruby
  # class C
  # end
  # C.singleton_class?                  #=> false
  # C.singleton_class.singleton_class?  #=> true
  # ```
  sig {returns(T::Boolean)}
  def singleton_class?(); end

  # Returns a string representing this module or class. For basic classes and
  # modules, this is the name. For singletons, we show information on the thing
  # we're attached to as well.
  #
  # Also aliased as:
  # [`inspect`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-inspect)
  sig {returns(String)}
  def to_s(); end

  # Prevents the current class from responding to calls to the named method.
  # Contrast this with `remove_method`, which deletes the method from the
  # particular class; Ruby will still search superclasses and mixed-in modules
  # for a possible receiver.
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) arguments are
  # converted to symbols.
  #
  # ```ruby
  # class Parent
  #   def hello
  #     puts "In parent"
  #   end
  # end
  # class Child < Parent
  #   def hello
  #     puts "In child"
  #   end
  # end
  #
  # c = Child.new
  # c.hello
  #
  # class Child
  #   remove_method :hello  # remove from child, still in parent
  # end
  # c.hello
  #
  # class Child
  #   undef_method :hello   # prevent any calls to 'hello'
  # end
  # c.hello
  # ```
  #
  # *produces:*
  #
  # ```
  # In child
  # In parent
  # prog.rb:23: undefined method `hello' for #<Child:0x401b3bb4> (NoMethodError)
  # ```
  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.self_type)
  end
  def undef_method(arg0); end

  # Import class refinements from *module* into the current class or module
  # definition.
  sig do
    params(
        arg0: Module,
    )
    .returns(T.self_type)
  end
  def using(arg0); end

  # Returns a string representing this module or class. For basic classes and
  # modules, this is the name. For singletons, we show information on the thing
  # we're attached to as well.
  #
  # Alias for:
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-to_s)
  sig {returns(String)}
  def inspect(); end

  # The first form is equivalent to
  # [`attr_reader`](https://docs.ruby-lang.org/en/2.7.0/Module.html#method-i-attr_reader).
  # The second form is equivalent to `attr_accessor(name)` but deprecated. The
  # last form is equivalent to `attr_reader(name)` but deprecated. Returns an
  # array of defined method names as symbols.
  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(NilClass)
  end
  def attr(*arg0); end

  # Returns an array of all modules used in the current scope. The ordering of
  # modules in the resulting array is not defined.
  #
  # ```ruby
  # module A
  #   refine Object do
  #   end
  # end
  #
  # module B
  #   refine Object do
  #   end
  # end
  #
  # using A
  # using B
  # p Module.used_modules
  # ```
  #
  # *produces:*
  #
  # ```ruby
  # [B, A]
  # ```
  def self.used_modules; end
end
