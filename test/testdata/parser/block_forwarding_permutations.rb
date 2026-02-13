# typed: false

# This file documents combinations of block forwarding.
# For more details, see test/prism_regression/call_forwarding_permutations.rb.
#
# 8 valid/error cases:
#   1. None                 - def f; bar; end
#   2. &                    - def f(&b); bar(&b); end
#   3. & (anonymous)        - def f(&); bar(&); end
#   4. ...                  - def f(...); bar(...); end
#   5. { }                  - def f; bar { }; end
#   6. ... + { }            - def f(...); bar(...) { }; end  (error: both block arg and literal)
#   7. & + { }              - def f(&b); bar(&b) { }; end   (error: both block arg and literal)
#   8. & (anonymous) + { }  - def f(&); bar(&) { }; end     (error: both block arg and literal)

# Case 1: None
def case1_none
  bar
end

# Case 2: Just & - Explicit block parameter
def case2_block_param(&block)
  bar(&block)
end

# Case 3: Anonymous block parameter
def case3_anonymous_block_param(&)
  bar(&)
end

# Case 4: Just ... - Forwarding only
def case4_forwarding(...)
  bar(...)
end

# Case 5: Just { } - Block literal only
def case5_block_literal
  bar { "block body" }
end

# Case 6: ... + { }
def case6_forwarding_and_literal(...)
  foo(...) { "literal block" }
#     ^^^ error: both block argument and literal block are passed
end

# Case 7: & + { }
def case7_block_pass_and_literal(&block)
  foo(&block) { "literal block" }
#     ^^^^^^ error: both block argument and literal block are passed
end

# Case 8: & (anonymous) + { }
def case8_anonymous_block_pass_and_literal(&)
  foo(&) { "literal block" }
#     ^ error: both block argument and literal block are passed
end
