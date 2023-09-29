# typed: __STDLIB_INTERNAL

# The [`Singleton`](https://docs.ruby-lang.org/en/2.7.0/Singleton.html) module
# implements the
# [`Singleton`](https://docs.ruby-lang.org/en/2.7.0/Singleton.html) pattern.
#
# ## Usage
#
# To use [`Singleton`](https://docs.ruby-lang.org/en/2.7.0/Singleton.html),
# include the module in your class.
#
# ```ruby
# class Klass
#    include Singleton
#    # ...
# end
# ```
#
# This ensures that only one instance of Klass can be created.
#
# ```ruby
# a,b  = Klass.instance, Klass.instance
#
# a == b
# # => true
#
# Klass.new
# # => NoMethodError - new is private ...
# ```
#
# The instance is created at upon the first call of Klass.instance().
#
# ```ruby
# class OtherKlass
#   include Singleton
#   # ...
# end
#
# ObjectSpace.each_object(OtherKlass){}
# # => 0
#
# OtherKlass.instance
# ObjectSpace.each_object(OtherKlass){}
# # => 1
# ```
#
# This behavior is preserved under inheritance and cloning.
#
# ## Implementation
#
# This above is achieved by:
#
# *   Making Klass.new and Klass.allocate private.
#
# *   Overriding Klass.inherited(sub\_klass) and Klass.clone() to ensure that
#     the [`Singleton`](https://docs.ruby-lang.org/en/2.7.0/Singleton.html)
#     properties are kept when inherited and cloned.
#
# *   Providing the Klass.instance() method that returns the same object each
#     time it is called.
#
# *   Overriding Klass.\_load(str) to call Klass.instance().
#
# *   Overriding Klass#clone and Klass#dup to raise TypeErrors to prevent
#     cloning or duping.
#
#
# ## [`Singleton`](https://docs.ruby-lang.org/en/2.7.0/Singleton.html) and [`Marshal`](https://docs.ruby-lang.org/en/2.7.0/Marshal.html)
#
# By default Singleton's #\_dump(depth) returns the empty string. Marshalling by
# default will strip state information, e.g. instance variables from the
# instance. Classes using
# [`Singleton`](https://docs.ruby-lang.org/en/2.7.0/Singleton.html) can provide
# custom \_load(str) and \_dump(depth) methods to retain some of the previous
# state of the instance.
#
# ```ruby
# require 'singleton'
#
# class Example
#   include Singleton
#   attr_accessor :keep, :strip
#   def _dump(depth)
#     # this strips the @strip information from the instance
#     Marshal.dump(@keep, depth)
#   end
#
#   def self._load(str)
#     instance.keep = Marshal.load(str)
#     instance
#   end
# end
#
# a = Example.instance
# a.keep = "keep this"
# a.strip = "get rid of this"
#
# stored_state = Marshal.dump(a)
#
# a.keep = nil
# a.strip = nil
# b = Marshal.load(stored_state)
# p a == b  #  => true
# p a.keep  #  => "keep this"
# p a.strip #  => nil
# ```
module Singleton
  module SingletonClassMethods
    has_attached_class!
    sig {returns(T.attached_class)}
    def instance; end

    sig { params(klass: T::Class[T.anything]).void }
    def self.__init__(klass); end

    private

    sig {void}
    def new; end
  end
  mixes_in_class_methods(SingletonClassMethods)
end
