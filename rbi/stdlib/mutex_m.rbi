# typed: __STDLIB_INTERNAL

# # mutex\_m.rb
#
# When 'mutex\_m' is required, any object that extends or includes
# [`Mutex_m`](https://docs.ruby-lang.org/en/2.6.0/Mutex_m.html) will be treated
# like a [`Mutex`](https://docs.ruby-lang.org/en/2.6.0/Mutex.html).
#
# Start by requiring the standard library
# [`Mutex_m`](https://docs.ruby-lang.org/en/2.6.0/Mutex_m.html):
#
# ```ruby
# require "mutex_m.rb"
# ```
#
# From here you can extend an object with
# [`Mutex`](https://docs.ruby-lang.org/en/2.6.0/Mutex.html) instance methods:
#
# ```ruby
# obj = Object.new
# obj.extend Mutex_m
# ```
#
# Or mixin [`Mutex_m`](https://docs.ruby-lang.org/en/2.6.0/Mutex_m.html) into
# your module to your class inherit
# [`Mutex`](https://docs.ruby-lang.org/en/2.6.0/Mutex.html) instance methods ---
# remember to call super() in your class initialize method.
#
# ```ruby
# class Foo
#   include Mutex_m
#   def initialize
#     # ...
#     super()
#   end
#   # ...
# end
# obj = Foo.new
# # this obj can be handled like Mutex
# ```
module Mutex_m
  def self.append_features(cl); end
  def self.define_aliases(cl); end
  def self.extend_object(obj); end
  def initialize(*args); end
  def mu_extended; end
  def mu_initialize; end
  # See
  # [`Mutex#lock`](https://docs.ruby-lang.org/en/2.6.0/Mutex.html#method-i-lock)
  def mu_lock; end
  # See
  # [`Mutex#locked?`](https://docs.ruby-lang.org/en/2.6.0/Mutex.html#method-i-locked-3F)
  def mu_locked?; end
  # See
  # [`Mutex#synchronize`](https://docs.ruby-lang.org/en/2.6.0/Mutex.html#method-i-synchronize)
  def mu_synchronize(&block); end
  # See
  # [`Mutex#try_lock`](https://docs.ruby-lang.org/en/2.6.0/Mutex.html#method-i-try_lock)
  def mu_try_lock; end
  # See
  # [`Mutex#unlock`](https://docs.ruby-lang.org/en/2.6.0/Mutex.html#method-i-unlock)
  def mu_unlock; end
  # See
  # [`Mutex#sleep`](https://docs.ruby-lang.org/en/2.6.0/Mutex.html#method-i-sleep)
  def sleep(timeout = nil); end
end
