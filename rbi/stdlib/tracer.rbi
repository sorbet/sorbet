# typed: __STDLIB_INTERNAL

# Outputs a source level execution trace of a Ruby program.
#
# It does this by registering an event handler with
# [`Kernel#set_trace_func`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html#method-i-set_trace_func)
# for processing incoming events. It also provides methods for filtering
# unwanted trace output (see
# [`Tracer.add_filter`](https://docs.ruby-lang.org/en/2.6.0/Tracer.html#method-c-add_filter),
# [`Tracer.on`](https://docs.ruby-lang.org/en/2.6.0/Tracer.html#method-c-on),
# and
# [`Tracer.off`](https://docs.ruby-lang.org/en/2.6.0/Tracer.html#method-c-off)).
#
# ## Example
#
# Consider the following Ruby script
#
# ```ruby
# class A
#   def square(a)
#     return a*a
#   end
# end
#
# a = A.new
# a.square(5)
# ```
#
# Running the above script using `ruby -r tracer example.rb` will output the
# following trace to STDOUT (Note you can also explicitly `require 'tracer'`)
#
# ```
# #0:<internal:lib/rubygems/custom_require>:38:Kernel:<: -
# #0:example.rb:3::-: class A
# #0:example.rb:3::C: class A
# #0:example.rb:4::-:   def square(a)
# #0:example.rb:7::E: end
# #0:example.rb:9::-: a = A.new
# #0:example.rb:10::-: a.square(5)
# #0:example.rb:4:A:>:   def square(a)
# #0:example.rb:5:A:-:     return a*a
# #0:example.rb:6:A:<:   end
#  |  |         | |  |
#  |  |         | |   ---------------------+ event
#  |  |         |  ------------------------+ class
#  |  |          --------------------------+ line
#  |   ------------------------------------+ filename
#   ---------------------------------------+ thread
# ```
#
# [`Symbol`](https://docs.ruby-lang.org/en/2.6.0/Symbol.html) table used for
# displaying incoming events:
#
# +}+
# :   call a C-language routine
# +{+
# :   return from a C-language routine
# +>+
# :   call a Ruby method
# `C`
# :   start a class or module definition
# `E`
# :   finish a class or module definition
# `-`
# :   execute code on a new line
# +^+
# :   raise an exception
# +<+
# :   return from a Ruby method
#
#
# ## Copyright
#
# by Keiju ISHITSUKA(keiju@ishitsuka.com)
class Tracer
  # [`Symbol`](https://docs.ruby-lang.org/en/2.6.0/Symbol.html) table used for
  # displaying trace information
  EVENT_SYMBOL = T.let(T.unsafe(nil), T::Hash[String, String])

  # Reference to singleton instance of
  # [`Tracer`](https://docs.ruby-lang.org/en/2.6.0/Tracer.html)
  Single = T.let(T.unsafe(nil), Tracer)

  VERSION = T.let(T.unsafe(nil), String)

  # Used to filter unwanted trace output
  #
  # Example which only outputs lines of code executed within the
  # [`Kernel`](https://docs.ruby-lang.org/en/2.6.0/Kernel.html) class:
  #
  # ```ruby
  # Tracer.add_filter do |event, file, line, id, binding, klass, *rest|
  #   "Kernel" == klass.to_s
  # end
  # ```
  def self.add_filter(p = _); end

  # display C-routine calls in trace output (defaults to false)
  def self.display_c_call; end

  # display C-routine calls in trace output (defaults to false)
  def self.display_c_call=(_); end

  # display C-routine calls in trace output (defaults to false)
  def self.display_c_call?; end

  # display process id in trace output (defaults to false)
  def self.display_process_id; end

  # display process id in trace output (defaults to false)
  def self.display_process_id=(_); end

  # display process id in trace output (defaults to false)
  def self.display_process_id?; end

  # display thread id in trace output (defaults to true)
  def self.display_thread_id; end

  # display thread id in trace output (defaults to true)
  def self.display_thread_id=(_); end

  # display thread id in trace output (defaults to true)
  def self.display_thread_id?; end

  # Disable tracing
  def self.off; end

  # Start tracing
  #
  # ### Example
  #
  # ```ruby
  # Tracer.on
  # # code to trace here
  # Tracer.off
  # ```
  #
  # You can also pass a block:
  #
  # ```ruby
  # Tracer.on {
  #   # trace everything in this block
  # }
  # ```
  def self.on; end

  # Register an event handler `p` which is called everytime a line in
  # `file_name` is executed.
  #
  # Example:
  #
  # ```ruby
  # Tracer.set_get_line_procs("example.rb", lambda { |line|
  #   puts "line number executed is #{line}"
  # })
  # ```
  def self.set_get_line_procs(file, p = _); end

  # output stream used to output trace (defaults to STDOUT)
  def self.stdout; end

  # output stream used to output trace (defaults to STDOUT)
  def self.stdout=(_); end

  # mutex lock used by tracer for displaying trace output
  def self.stdout_mutex; end

  # display additional debug information (defaults to false)
  def self.verbose; end

  # display additional debug information (defaults to false)
  def self.verbose=(_); end

  # display additional debug information (defaults to false)
  def self.verbose?; end
end
