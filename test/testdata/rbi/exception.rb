# typed: strict

StandardError.new
StandardError.new(nil)
StandardError.new('msg')
StandardError.new(:msg)
ex = StandardError.new("bees")
RuntimeError.new(ex)

SystemCallError.new # error: Not enough arguments provided for method `SystemCallError.new`. Expected: `1`, got: `0`
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
