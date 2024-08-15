# typed: true

 foo {} # empty inline block
#^^^ error: Method `foo` does not exist on `T.class_of(<root>)`

 foo do # empty do-end block
#^^^ error: Method `foo` does not exist on `T.class_of(<root>)`
 end

 foo { "inline block" }
#^^^ error: Method `foo` does not exist on `T.class_of(<root>)`

 foo do
#^^^ error: Method `foo` does not exist on `T.class_of(<root>)`
   "do-end block"
 end

 foo { |positional, kwarg:, &block| "inline block with params" }
#^^^ error: Method `foo` does not exist on `T.class_of(<root>)`

 foo do |positional, kwarg:, &block|
#^^^ error: Method `foo` does not exist on `T.class_of(<root>)`
   "inline block with params"
 end
