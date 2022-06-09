# typed: __STDLIB_INTERNAL

# A libffi wrapper for Ruby.
#
# ## Description
#
# [`Fiddle`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html) is an extension to
# translate a foreign function interface (FFI) with ruby.
#
# It wraps [libffi](http://sourceware.org/libffi/), a popular C library which
# provides a portable interface that allows code written in one language to call
# code written in another language.
#
# ## Example
#
# Here we will use
# [`Fiddle::Function`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Function.html)
# to wrap [floor(3) from libm](http://linux.die.net/man/3/floor)
#
# ```ruby
# require 'fiddle'
#
# libm = Fiddle.dlopen('/lib/libm.so.6')
#
# floor = Fiddle::Function.new(
#   libm['floor'],
#   [Fiddle::TYPE_DOUBLE],
#   Fiddle::TYPE_DOUBLE
# )
#
# puts floor.call(3.14159) #=> 3.0
# ```
module Fiddle
  # [`ALIGN_CHAR`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#ALIGN_CHAR)
  #
  # The alignment size of a char
  ALIGN_CHAR = T.let(T.unsafe(nil), Integer)

  # [`ALIGN_DOUBLE`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#ALIGN_DOUBLE)
  #
  # The alignment size of a double
  ALIGN_DOUBLE = T.let(T.unsafe(nil), Integer)

  # [`ALIGN_FLOAT`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#ALIGN_FLOAT)
  #
  # The alignment size of a float
  ALIGN_FLOAT = T.let(T.unsafe(nil), Integer)

  # [`ALIGN_INT`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#ALIGN_INT)
  #
  # The alignment size of an int
  ALIGN_INT = T.let(T.unsafe(nil), Integer)

  # [`ALIGN_INTPTR_T`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#ALIGN_INTPTR_T)
  #
  # The alignment size of a intptr\_t
  ALIGN_INTPTR_T = T.let(T.unsafe(nil), Integer)

  # [`ALIGN_LONG`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#ALIGN_LONG)
  #
  # The alignment size of a long
  ALIGN_LONG = T.let(T.unsafe(nil), Integer)

  # [`ALIGN_LONG_LONG`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#ALIGN_LONG_LONG)
  #
  # The alignment size of a long long
  ALIGN_LONG_LONG = T.let(T.unsafe(nil), Integer)

  # [`ALIGN_PTRDIFF_T`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#ALIGN_PTRDIFF_T)
  #
  # The alignment size of a ptrdiff\_t
  ALIGN_PTRDIFF_T = T.let(T.unsafe(nil), Integer)

  # [`ALIGN_SHORT`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#ALIGN_SHORT)
  #
  # The alignment size of a short
  ALIGN_SHORT = T.let(T.unsafe(nil), Integer)

  # [`ALIGN_SIZE_T`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#ALIGN_SIZE_T)
  #
  # The alignment size of a size\_t
  ALIGN_SIZE_T = T.let(T.unsafe(nil), Integer)

  # [`ALIGN_SSIZE_T`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#ALIGN_SSIZE_T)
  #
  # The alignment size of a ssize\_t
  ALIGN_SSIZE_T = T.let(T.unsafe(nil), Integer)

  # [`ALIGN_UINTPTR_T`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#ALIGN_UINTPTR_T)
  #
  # The alignment size of a uintptr\_t
  ALIGN_UINTPTR_T = T.let(T.unsafe(nil), Integer)

  # [`ALIGN_VOIDP`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#ALIGN_VOIDP)
  #
  # The alignment size of a void\*
  ALIGN_VOIDP = T.let(T.unsafe(nil), Integer)

  # [`BUILD_RUBY_PLATFORM`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#BUILD_RUBY_PLATFORM)
  #
  # Platform built against (i.e. "x86\_64-linux", etc.)
  #
  # See also RUBY\_PLATFORM
  BUILD_RUBY_PLATFORM = T.let(T.unsafe(nil), String)

  # [`RUBY_FREE`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#RUBY_FREE)
  #
  # Address of the ruby\_xfree() function
  RUBY_FREE = T.let(T.unsafe(nil), Integer)

  # [`SIZEOF_CHAR`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#SIZEOF_CHAR)
  #
  # size of a char
  SIZEOF_CHAR = T.let(T.unsafe(nil), Integer)

  # [`SIZEOF_DOUBLE`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#SIZEOF_DOUBLE)
  #
  # size of a double
  SIZEOF_DOUBLE = T.let(T.unsafe(nil), Integer)

  # [`SIZEOF_FLOAT`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#SIZEOF_FLOAT)
  #
  # size of a float
  SIZEOF_FLOAT = T.let(T.unsafe(nil), Integer)

  # [`SIZEOF_INT`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#SIZEOF_INT)
  #
  # size of an int
  SIZEOF_INT = T.let(T.unsafe(nil), Integer)

  # [`SIZEOF_INTPTR_T`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#SIZEOF_INTPTR_T)
  #
  # size of a intptr\_t
  SIZEOF_INTPTR_T = T.let(T.unsafe(nil), Integer)

  # [`SIZEOF_LONG`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#SIZEOF_LONG)
  #
  # size of a long
  SIZEOF_LONG = T.let(T.unsafe(nil), Integer)

  # [`SIZEOF_LONG_LONG`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#SIZEOF_LONG_LONG)
  #
  # size of a long long
  SIZEOF_LONG_LONG = T.let(T.unsafe(nil), Integer)

  # [`SIZEOF_PTRDIFF_T`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#SIZEOF_PTRDIFF_T)
  #
  # size of a ptrdiff\_t
  SIZEOF_PTRDIFF_T = T.let(T.unsafe(nil), Integer)

  # [`SIZEOF_SHORT`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#SIZEOF_SHORT)
  #
  # size of a short
  SIZEOF_SHORT = T.let(T.unsafe(nil), Integer)

  # [`SIZEOF_SIZE_T`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#SIZEOF_SIZE_T)
  #
  # size of a size\_t
  SIZEOF_SIZE_T = T.let(T.unsafe(nil), Integer)

  # [`SIZEOF_SSIZE_T`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#SIZEOF_SSIZE_T)
  #
  # size of a ssize\_t
  SIZEOF_SSIZE_T = T.let(T.unsafe(nil), Integer)

  # [`SIZEOF_UINTPTR_T`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#SIZEOF_UINTPTR_T)
  #
  # size of a uintptr\_t
  SIZEOF_UINTPTR_T = T.let(T.unsafe(nil), Integer)

  # [`SIZEOF_VOIDP`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#SIZEOF_VOIDP)
  #
  # size of a void\*
  SIZEOF_VOIDP = T.let(T.unsafe(nil), Integer)

  # [`TYPE_CHAR`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#TYPE_CHAR)
  #
  # C type - char
  TYPE_CHAR = T.let(T.unsafe(nil), Integer)

  # [`TYPE_DOUBLE`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#TYPE_DOUBLE)
  #
  # C type - double
  TYPE_DOUBLE = T.let(T.unsafe(nil), Integer)

  # [`TYPE_FLOAT`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#TYPE_FLOAT)
  #
  # C type - float
  TYPE_FLOAT = T.let(T.unsafe(nil), Integer)

  # [`TYPE_INT`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#TYPE_INT)
  #
  # C type - int
  TYPE_INT = T.let(T.unsafe(nil), Integer)

  # [`TYPE_INTPTR_T`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#TYPE_INTPTR_T)
  #
  # C type - intptr\_t
  TYPE_INTPTR_T = T.let(T.unsafe(nil), Integer)

  # [`TYPE_LONG`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#TYPE_LONG)
  #
  # C type - long
  TYPE_LONG = T.let(T.unsafe(nil), Integer)

  # [`TYPE_LONG_LONG`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#TYPE_LONG_LONG)
  #
  # C type - long long
  TYPE_LONG_LONG = T.let(T.unsafe(nil), Integer)

  # [`TYPE_PTRDIFF_T`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#TYPE_PTRDIFF_T)
  #
  # C type - ptrdiff\_t
  TYPE_PTRDIFF_T = T.let(T.unsafe(nil), Integer)

  # [`TYPE_SHORT`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#TYPE_SHORT)
  #
  # C type - short
  TYPE_SHORT = T.let(T.unsafe(nil), Integer)

  # [`TYPE_SIZE_T`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#TYPE_SIZE_T)
  #
  # C type - size\_t
  TYPE_SIZE_T = T.let(T.unsafe(nil), Integer)

  # [`TYPE_SSIZE_T`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#TYPE_SSIZE_T)
  #
  # C type - ssize\_t
  TYPE_SSIZE_T = T.let(T.unsafe(nil), Integer)

  # [`TYPE_UINTPTR_T`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#TYPE_UINTPTR_T)
  #
  # C type - uintptr\_t
  TYPE_UINTPTR_T = T.let(T.unsafe(nil), Integer)

  # [`TYPE_VOID`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#TYPE_VOID)
  #
  # C type - void
  TYPE_VOID = T.let(T.unsafe(nil), Integer)

  # [`TYPE_VOIDP`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#TYPE_VOIDP)
  #
  # C type - void\*
  TYPE_VOIDP = T.let(T.unsafe(nil), Integer)

  # Creates a new handler that opens `library`, and returns an instance of
  # [`Fiddle::Handle`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Handle.html).
  #
  # If `nil` is given for the `library`, Fiddle::Handle::DEFAULT is used, which
  # is the equivalent to RTLD\_DEFAULT. See `man 3 dlopen` for more.
  #
  # ```ruby
  # lib = Fiddle.dlopen(nil)
  # ```
  #
  # The default is dependent on OS, and provide a handle for all libraries
  # already loaded. For example, in most cases you can use this to access `libc`
  # functions, or ruby functions like `rb_str_new`.
  #
  # See
  # [`Fiddle::Handle.new`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Handle.html#method-c-new)
  # for more.
  def self.dlopen(library); end

  # Returns the hexadecimal representation of a memory pointer address `addr`
  #
  # Example:
  #
  # ```
  # lib = Fiddle.dlopen('/lib64/libc-2.15.so')
  # => #<Fiddle::Handle:0x00000001342460>
  #
  # lib['strcpy'].to_s(16)
  # => "7f59de6dd240"
  #
  # Fiddle.dlunwrap(Fiddle.dlwrap(lib['strcpy'].to_s(16)))
  # => "7f59de6dd240"
  # ```
  def self.dlunwrap(_); end

  # Returns a memory pointer of a function's hexadecimal address location `val`
  #
  # Example:
  #
  # ```
  # lib = Fiddle.dlopen('/lib64/libc-2.15.so')
  # => #<Fiddle::Handle:0x00000001342460>
  #
  # Fiddle.dlwrap(lib['strcpy'].to_s(16))
  # => 25522520
  # ```
  def self.dlwrap(_); end

  # Free the memory at address `addr`
  def self.free(_); end

  # Returns the last `Error` of the current executing `Thread` or nil if none
  def self.last_error; end

  # Sets the last `Error` of the current executing `Thread` to `error`
  def self.last_error=(error); end

  # Allocate `size` bytes of memory and return the integer memory address for
  # the allocated memory.
  def self.malloc(_); end

  # Change the size of the memory allocated at the memory location `addr` to
  # `size` bytes. Returns the memory address of the reallocated memory, which
  # may be different than the address passed in.
  def self.realloc(_, _); end
end

# ## Description
#
# An FFI closure wrapper, for handling callbacks.
#
# ## Example
#
# ```ruby
# closure = Class.new(Fiddle::Closure) {
#   def call
#     10
#   end
# }.new(Fiddle::TYPE_INT, [])
#    #=> #<#<Class:0x0000000150d308>:0x0000000150d240>
# func = Fiddle::Function.new(closure, [], Fiddle::TYPE_INT)
#    #=> #<Fiddle::Function:0x00000001516e58>
# func.call
#    #=> 10
# ```
class Fiddle::Closure
  def initialize(*_); end

  # arguments of the FFI closure
  def args; end

  # the C type of the return of the FFI closure
  def ctype; end

  # Returns the memory address for this closure
  def to_i; end
end

# A mixin that provides methods for parsing C struct and prototype signatures.
#
# ## Example
#
# ```ruby
# require 'fiddle/import'
#
# include Fiddle::CParser
#   #=> Object
#
# parse_ctype('int')
#   #=> Fiddle::TYPE_INT
#
# parse_struct_signature(['int i', 'char c'])
#   #=> [[Fiddle::TYPE_INT, Fiddle::TYPE_CHAR], ["i", "c"]]
#
# parse_signature('double sum(double, double)')
#   #=> ["sum", Fiddle::TYPE_DOUBLE, [Fiddle::TYPE_DOUBLE, Fiddle::TYPE_DOUBLE]]
# ```
module Fiddle::CParser
  # Given a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) of C
  # type `ty`, returns the corresponding
  # [`Fiddle`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html) constant.
  #
  # `ty` can also accept an
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of C type Strings,
  # and will be returned in a corresponding
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html).
  #
  # If [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) `tymap` is
  # provided, `ty` is expected to be the key, and the value will be the C type
  # to be looked up.
  #
  # Example:
  #
  # ```ruby
  # require 'fiddle/import'
  #
  # include Fiddle::CParser
  #   #=> Object
  #
  # parse_ctype('int')
  #   #=> Fiddle::TYPE_INT
  #
  # parse_ctype('double diff')
  #   #=> Fiddle::TYPE_DOUBLE
  #
  # parse_ctype('unsigned char byte')
  #   #=> -Fiddle::TYPE_CHAR
  #
  # parse_ctype('const char* const argv[]')
  #   #=> -Fiddle::TYPE_VOIDP
  # ```
  def parse_ctype(ty, tymap=nil); end

  # Parses a C prototype signature
  #
  # If [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) `tymap` is
  # provided, the return value and the arguments from the `signature` are
  # expected to be keys, and the value will be the C type to be looked up.
  #
  # Example:
  #
  # ```ruby
  # require 'fiddle/import'
  #
  # include Fiddle::CParser
  #   #=> Object
  #
  # parse_signature('double sum(double, double)')
  #   #=> ["sum", Fiddle::TYPE_DOUBLE, [Fiddle::TYPE_DOUBLE, Fiddle::TYPE_DOUBLE]]
  #
  # parse_signature('void update(void (*cb)(int code))')
  #   #=> ["update", Fiddle::TYPE_VOID, [Fiddle::TYPE_VOIDP]]
  #
  # parse_signature('char (*getbuffer(void))[80]')
  #   #=> ["getbuffer", Fiddle::TYPE_VOIDP, []]
  # ```
  def parse_signature(signature, tymap=nil); end

  # Parses a C struct's members
  #
  # Example:
  #
  # ```ruby
  # require 'fiddle/import'
  #
  # include Fiddle::CParser
  #   #=> Object
  #
  # parse_struct_signature(['int i', 'char c'])
  #   #=> [[Fiddle::TYPE_INT, Fiddle::TYPE_CHAR], ["i", "c"]]
  #
  # parse_struct_signature(['char buffer[80]'])
  #   #=> [[[Fiddle::TYPE_CHAR, 80]], ["buffer"]]
  # ```
  def parse_struct_signature(signature, tymap=nil); end
end

# Extends
# [`Fiddle::Closure`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Closure.html)
# to allow for building the closure in a block
class Fiddle::Closure::BlockCaller < ::Fiddle::Closure
  def initialize(ctype, args, abi = _, &block); end

  # Calls the constructed
  # [`BlockCaller`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Closure/BlockCaller.html),
  # with `args`
  #
  # For an example see
  # [`Fiddle::Closure::BlockCaller.new`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Closure/BlockCaller.html#method-c-new)
  def call(*args); end
end

# standard dynamic load exception
class Fiddle::DLError < ::StandardError
end

# ## Description
#
# A representation of a C function
#
# ## Examples
#
# ### 'strcpy'
#
# ```
# @libc = Fiddle.dlopen "/lib/libc.so.6"
#    #=> #<Fiddle::Handle:0x00000001d7a8d8>
# f = Fiddle::Function.new(
#   @libc['strcpy'],
#   [Fiddle::TYPE_VOIDP, Fiddle::TYPE_VOIDP],
#   Fiddle::TYPE_VOIDP)
#    #=> #<Fiddle::Function:0x00000001d8ee00>
# buff = "000"
#    #=> "000"
# str = f.call(buff, "123")
#    #=> #<Fiddle::Pointer:0x00000001d0c380 ptr=0x000000018a21b8 size=0 free=0x00000000000000>
# str.to_s
# => "123"
# ```
#
# ### ABI check
#
# ```ruby
# @libc = Fiddle.dlopen "/lib/libc.so.6"
#    #=> #<Fiddle::Handle:0x00000001d7a8d8>
# f = Fiddle::Function.new(@libc['strcpy'], [TYPE_VOIDP, TYPE_VOIDP], TYPE_VOIDP)
#    #=> #<Fiddle::Function:0x00000001d8ee00>
# f.abi == Fiddle::Function::DEFAULT
#    #=> true
# ```
class Fiddle::Function
  # [`DEFAULT`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Function.html#DEFAULT)
  #
  # Default ABI
  DEFAULT = T.let(T.unsafe(nil), Integer)

  def initialize(*_); end

  # The ABI of the
  # [`Function`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Function.html).
  def abi; end

  # Calls the constructed
  # [`Function`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Function.html), with
  # `args`. Caller must ensure the underlying function is called in a
  # thread-safe manner if running in a multi-threaded process.
  #
  # For an example see
  # [`Fiddle::Function`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Function.html)
  def call(*_); end

  # The name of this function
  def name; end

  # The address of this function
  def ptr; end

  # The integer memory location of this function
  def to_i; end
end

# The [`Fiddle::Handle`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Handle.html)
# is the manner to access the dynamic library
#
# ## Example
#
# ### Setup
#
# ```
# libc_so = "/lib64/libc.so.6"
# => "/lib64/libc.so.6"
# @handle = Fiddle::Handle.new(libc_so)
# => #<Fiddle::Handle:0x00000000d69ef8>
# ```
#
# ### Setup, with flags
#
# ```
# libc_so = "/lib64/libc.so.6"
# => "/lib64/libc.so.6"
# @handle = Fiddle::Handle.new(libc_so, Fiddle::RTLD_LAZY | Fiddle::RTLD_GLOBAL)
# => #<Fiddle::Handle:0x00000000d69ef8>
# ```
#
# See
# [`RTLD_LAZY`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Handle.html#RTLD_LAZY)
# and
# [`RTLD_GLOBAL`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Handle.html#RTLD_GLOBAL)
#
# ### Addresses to symbols
#
# ```
# strcpy_addr = @handle['strcpy']
# => 140062278451968
# ```
#
# or
#
# ```
# strcpy_addr = @handle.sym('strcpy')
# => 140062278451968
# ```
class Fiddle::Handle
  # [`DEFAULT`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Handle.html#DEFAULT)
  #
  # A predefined pseudo-handle of RTLD\_DEFAULT
  #
  # Which will find the first occurrence of the desired symbol using the default
  # library search order
  DEFAULT = T.let(T.unsafe(nil), Fiddle::Handle)

  # [`NEXT`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Handle.html#NEXT)
  #
  # A predefined pseudo-handle of RTLD\_NEXT
  #
  # Which will find the next occurrence of a function in the search order after
  # the current library.
  NEXT = T.let(T.unsafe(nil), Fiddle::Handle)

  # [`RTLD_GLOBAL`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Handle.html#RTLD_GLOBAL)
  #
  # rtld
  # [`Fiddle::Handle`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Handle.html)
  # flag.
  #
  # The symbols defined by this library will be made available for symbol
  # resolution of subsequently loaded libraries.
  RTLD_GLOBAL = T.let(T.unsafe(nil), Integer)

  # [`RTLD_LAZY`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Handle.html#RTLD_LAZY)
  #
  # rtld
  # [`Fiddle::Handle`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Handle.html)
  # flag.
  #
  # Perform lazy binding. Only resolve symbols as the code that references them
  # is executed. If the  symbol is never referenced, then it is never resolved.
  # (Lazy binding is only performed for function references; references to
  # variables are always immediately bound when the library is loaded.)
  RTLD_LAZY = T.let(T.unsafe(nil), Integer)

  # [`RTLD_NOW`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Handle.html#RTLD_NOW)
  #
  # rtld
  # [`Fiddle::Handle`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Handle.html)
  # flag.
  #
  # If this value is specified or the environment variable LD\_BIND\_NOW is set
  # to a nonempty string, all undefined symbols in the library are resolved
  # before
  # [`Fiddle.dlopen`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#method-c-dlopen)
  # returns. If this cannot be done an error is returned.
  RTLD_NOW = T.let(T.unsafe(nil), Integer)

  def initialize(*_); end

  # Get the address as an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) for the
  # function named `name`.
  def [](_); end

  # Close this handle.
  #
  # Calling close more than once will raise a
  # [`Fiddle::DLError`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/DLError.html)
  # exception.
  def close; end

  # Returns `true` if dlclose() will be called when this handle is garbage
  # collected.
  #
  # See man(3) dlclose() for more info.
  def close_enabled?; end

  # Disable a call to dlclose() when this handle is garbage collected.
  def disable_close; end

  # Enable a call to dlclose() when this handle is garbage collected.
  def enable_close; end

  # Get the address as an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) for the
  # function named `name`.
  def sym(_); end

  # Returns the memory address for this handle.
  def to_i; end

  # Get the address as an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) for the
  # function named `name`. The function is searched via dlsym on RTLD\_NEXT.
  #
  # See man(3) dlsym() for more info.
  def self.[](_); end

  # Get the address as an
  # [`Integer`](https://docs.ruby-lang.org/en/2.7.0/Integer.html) for the
  # function named `name`.
  def self.sym(_); end
end

# A DSL that provides the means to dynamically load libraries and build modules
# around them including calling extern functions within the C library that has
# been loaded.
#
# ## Example
#
# ```ruby
# require 'fiddle'
# require 'fiddle/import'
#
# module LibSum
#   extend Fiddle::Importer
#   dlload './libsum.so'
#   extern 'double sum(double*, int)'
#   extern 'double split(double)'
# end
# ```
module Fiddle::Importer
  # Returns the function mapped to `name`, that was created by either
  # [`Fiddle::Importer.extern`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Importer.html#method-i-extern)
  # or
  # [`Fiddle::Importer.bind`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Importer.html#method-i-bind)
  def [](name); end

  # Creates a global method from the given C `signature` using the given `opts`
  # as bind parameters with the given block.
  def bind(signature, *opts, &blk); end

  # Returns a new closure wrapper for the `name` function.
  #
  # *   `ctype` is the return type of the function
  # *   `argtype` is an
  #     [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of arguments,
  #     passed to the callback function
  # *   `call_type` is the abi of the closure
  # *   `block` is passed to the callback
  #
  #
  # See
  # [`Fiddle::Closure`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Closure.html)
  def bind_function(name, ctype, argtype, call_type = nil, &block); end

  # Creates a class to wrap the C struct with the value `ty`
  #
  # See also
  # [`Fiddle::Importer.struct`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Importer.html#method-i-struct)
  #
  # Also aliased as:
  # [`value`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Importer.html#method-i-value)
  def create_value(ty, val=nil); end

  # Creates an array of handlers for the given `libs`, can be an instance of
  # [`Fiddle::Handle`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Handle.html),
  # [`Fiddle::Importer`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Importer.html),
  # or will create a new instance of
  # [`Fiddle::Handle`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Handle.html)
  # using
  # [`Fiddle.dlopen`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#method-c-dlopen)
  #
  # Raises a DLError if the library cannot be loaded.
  #
  # See
  # [`Fiddle.dlopen`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html#method-c-dlopen)
  def dlload(*libs); end

  # Creates a global method from the given C `signature`.
  def extern(signature, *opts); end

  # The
  # [`Fiddle::CompositeHandler`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/CompositeHandler.html)
  # instance
  #
  # Will raise an error if no handlers are open.
  def handler; end

  # Returns a new
  # [`Fiddle::Function`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Function.html)
  # instance at the memory address of the given `name` function.
  #
  # Raises a DLError if the `name` doesn't exist.
  #
  # *   `argtype` is an
  #     [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of arguments,
  #     passed to the `name` function.
  # *   `ctype` is the return type of the function
  # *   `call_type` is the ABI of the function
  #
  #
  # See also Fiddle:Function.new
  #
  # See
  # [`Fiddle::CompositeHandler.sym`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/CompositeHandler.html#method-i-sym)
  # and Fiddle::Handler.sym
  def import_function(name, ctype, argtype, call_type = nil); end

  # Returns a new
  # [`Fiddle::Pointer`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Pointer.html)
  # instance at the memory address of the given `name` symbol.
  #
  # Raises a DLError if the `name` doesn't exist.
  #
  # See
  # [`Fiddle::CompositeHandler.sym`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/CompositeHandler.html#method-i-sym)
  # and
  # [`Fiddle::Handle.sym`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Handle.html#method-c-sym)
  def import_symbol(name); end

  # Returns a new instance of the C struct with the value `ty` at the `addr`
  # address.
  def import_value(ty, addr); end

  # Returns the sizeof `ty`, using
  # [`Fiddle::Importer.parse_ctype`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/CParser.html#method-i-parse_ctype)
  # to determine the C type and the appropriate
  # [`Fiddle`](https://docs.ruby-lang.org/en/2.7.0/Fiddle.html) constant.
  def sizeof(ty); end

  # Creates a class to wrap the C struct described by `signature`.
  #
  # ```ruby
  # MyStruct = struct ['int i', 'char c']
  # ```
  def struct(signature); end

  # Sets the type alias for `alias_type` as `orig_type`
  def typealias(alias_type, orig_type); end

  # Creates a class to wrap the C union described by `signature`.
  #
  # ```ruby
  # MyUnion = union ['int i', 'char c']
  # ```
  def union(signature); end

  # Alias for:
  # [`create_value`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Importer.html#method-i-create_value)
  def value(ty, val=nil); end
end

# [`Fiddle::Pointer`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Pointer.html)
# is a class to handle C pointers
class Fiddle::Pointer
  def initialize(*_); end

  # Returns a new pointer instance that has been advanced `n` bytes.
  def +(_); end

  # Returns a new
  # [`Fiddle::Pointer`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Pointer.html)
  # instance that is a dereferenced pointer for this pointer.
  #
  # Analogous to the star operator in C.
  def +@; end

  # Returns a new pointer instance that has been moved back `n` bytes.
  def -(_); end

  # Returns a new
  # [`Fiddle::Pointer`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Pointer.html)
  # instance that is a reference pointer for this pointer.
  #
  # Analogous to the ampersand operator in C.
  def -@; end

  # Returns -1 if less than, 0 if equal to, 1 if greater than `other`.
  #
  # Returns nil if `ptr` cannot be compared to `other`.
  def <=>(_); end

  # Returns true if `other` wraps the same pointer, otherwise returns false.
  def ==(_); end

  # Returns integer stored at *index*.
  #
  # If *start* and *length* are given, a string containing the bytes from
  # *start* of *length* will be returned.
  def [](*_); end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the value at `index`
  # to `int`.
  #
  # Or, set the memory at `start` until `length` with the contents of `string`,
  # the memory from `dl_cptr`, or the memory pointed at by the memory address
  # `addr`.
  def []=(*_); end

  # Returns true if `other` wraps the same pointer, otherwise returns false.
  def eql?(_); end

  # Get the free function for this pointer.
  #
  # Returns a new instance of
  # [`Fiddle::Function`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Function.html).
  #
  # See
  # [`Fiddle::Function.new`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Function.html#method-c-new)
  def free; end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the free function for
  # this pointer to `function` in the given
  # [`Fiddle::Function`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Function.html).
  def free=(_); end

  # Returns a string formatted with an easily readable representation of the
  # internal state of the pointer.
  def inspect; end

  # Returns `true` if this is a null pointer.
  def null?; end

  # Returns a new
  # [`Fiddle::Pointer`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Pointer.html)
  # instance that is a dereferenced pointer for this pointer.
  #
  # Analogous to the star operator in C.
  def ptr; end

  # Returns a new
  # [`Fiddle::Pointer`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Pointer.html)
  # instance that is a reference pointer for this pointer.
  #
  # Analogous to the ampersand operator in C.
  def ref; end

  # Get the size of this pointer.
  def size; end

  # [`Set`](https://docs.ruby-lang.org/en/2.7.0/Set.html) the size of this
  # pointer to `size`
  def size=(_); end

  # Returns the integer memory location of this pointer.
  def to_i; end

  # Returns the integer memory location of this pointer.
  def to_int; end

  # Returns the pointer contents as a string.
  #
  # When called with no arguments, this method will return the contents until
  # the first NULL byte.
  #
  # When called with `len`, a string of `len` bytes will be returned.
  #
  # See
  # [`to_str`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Pointer.html#method-i-to_str)
  def to_s(*_); end

  # Returns the pointer contents as a string.
  #
  # When called with no arguments, this method will return the contents with the
  # length of this pointer's `size`.
  #
  # When called with `len`, a string of `len` bytes will be returned.
  #
  # See
  # [`to_s`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Pointer.html#method-i-to_s)
  def to_str(*_); end

  # Cast this pointer to a ruby object.
  def to_value; end

  # Get the underlying pointer for ruby object `val` and return it as a
  # [`Fiddle::Pointer`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Pointer.html)
  # object.
  def self.[](_); end

  # Allocate `size` bytes of memory and associate it with an optional `freefunc`
  # that will be called when the pointer is garbage collected.
  #
  # `freefunc` must be an address pointing to a function or an instance of
  # [`Fiddle::Function`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Function.html)
  def self.malloc(*_); end

  # Get the underlying pointer for ruby object `val` and return it as a
  # [`Fiddle::Pointer`](https://docs.ruby-lang.org/en/2.7.0/Fiddle/Pointer.html)
  # object.
  def self.to_ptr(_); end
end
