# typed: false

# This file documents combinations of block forwarding.
#
# 7 valid/error cases:
#   1. None           - def f; bar; end
#   2. Just & (named) - def f(&b); bar(&b); end
#   2a. Just & (anon) - def f(&); bar(&); end
#   3. Just ...       - def f(...); bar(...); end
#   4. Just { }       - def f; bar { }; end
#   5. ... + { }      - def f(...); bar(...) { }; end  (error: both block arg and literal)
#   6. & + { } (named)- def f(&b); bar(&b) { }; end   (error: both block arg and literal)
#   6a. & + { } (anon)- def f(&); bar(&) { }; end     (error: both block arg and literal)

# Case 1: None
def case1_none
  bar
end

# Case 2: Just & (named)
def case2_block_param(&block)
  bar(&block)
end

# Case 2a: Just & (anonymous)
def case2a_anonymous_block_param(&)
  bar(&)
end

# Case 3: Just ...
def case3_forwarding(...)
  bar(...)
end

# Case 4: Just { }
def case4_block_literal
  bar { "block body" }
end

# Case 5: ... + { }
def case5_forwarding_and_literal(...)
  foo(...) { "literal block" }
#     ^^^ error: both block argument and literal block are passed
end

# Case 6: & + { } (named)
def case6_block_pass_and_literal(&block)
  foo(&block) { "literal block" }
#     ^^^^^^ error: both block argument and literal block are passed
end

# Case 6a: & + { } (anonymous)
def case6a_anonymous_block_pass_and_literal(&)
  foo(&) { "literal block" }
#     ^ error: both block argument and literal block are passed
end
