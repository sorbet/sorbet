# typed: false

 foo {} # empty inline block

 foo do # empty do-end block
 end

 foo { "inline block" }

 foo do
   "do-end block"
 end

 foo { |positional, kwarg:, &block| "inline block with params" }

 foo do |positional, kwarg:, &block|
   "inline block with params"
 end

foo do |positional, (multi, target)|
  "block with multi-target node in parameter list"
end

 foo(&forwarded_block)

 foo&.bar {}
