# typed: strict

StandardError.new
StandardError.new(nil)
StandardError.new('msg')
StandardError.new(:msg)
ex = StandardError.new("bees")
RuntimeError.new(ex)

SystemCallError.new # error: Not enough arguments provided for method `SystemCallError#initialize (overload.1)`. Expected: `1`, got: `0`
SystemCallError.new(1)
SystemCallError.new(nil)
SystemCallError.new("message")
SystemCallError.new("message", 1)
SystemCallError.new("message", nil)
SystemCallError.new("message", "func") # error: Expected `T.nilable(Integer)` but found `String("func")` for argument `errno`
SystemCallError.new("message", 1, "func")
SystemCallError.new("message", nil, "func")
Errno::ENOENT.new
Errno::ENOENT.new(1) # error: Expected `T.nilable(String)` but found `Integer(1)` for argument `message`
Errno::ENOENT.new("message")
Errno::ENOENT.new("message", "func")
T.reveal_type(SystemCallError.new(nil).errno) # error: Revealed type: `T.nilable(Integer)`
T.reveal_type(Errno::ENOENT.new.errno) # error: Revealed type: `Integer`

T.reveal_type(ex.set_backtrace(caller)) # error: Revealed type: `T::Array[String]`
T.reveal_type(ex.set_backtrace("foo.rb:1")) # error: Revealed type: `T::Array[String]`
ex.set_backtrace(nil)
T.reveal_type(ex.set_backtrace(caller_locations)) # error: Revealed type: `T::Array[Thread::Backtrace::Location]`
T.reveal_type(ex.set_backtrace(ex.backtrace_locations || ex.backtrace)) # error: Revealed type: `T.nilable(T::Array[T.any(String, Thread::Backtrace::Location)])`
ex.set_backtrace([1]) # error: Expected `T.nilable(T.any(T::Array[String], String))` but found `[Integer(1)]` for argument `arg0`
