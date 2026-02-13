# typed: false

# This file tests combinations of block forwarding and literal blocks.
#
# Summary of 12 combinations:
#   Valid:
#     1. None                 - def f; bar; end
#     2. &                    - def f(&b); bar(&b); end
#     3. & (anonymous)        - def f(&); bar(&); end
#     4. ...                  - def f(...); bar(...); end
#     5. { }                  - def f; bar { }; end
#     6. ... + { }            - def f(...); bar(...) { }; end
#     7. & + { }              - def f(&b); bar(&b) { }; end
#     8. & (anonymous) + { }  - def f(&); bar(&) { }; end
#   Error cases (definition - cannot be tested, see below):
#     9. & + ...              - def f(&b, ...); end
#    10. & (anonymous) + ...  - def f(&, ...); end
#    11. & + ... + { }        - def f(&b, ...); bar(...) { }; end
#    12. & (anonymous) + ... + { } - def f(&, ...); bar(...) { }; end

# Cases 9-12 are excluded from this test.
#
# These cases have `&` and `...` together in the definition, which is a parser
# error. The error recovery is fundamentally different between parsers:
#   - Whitequark: Aggressive recovery destroys the entire file from error point
#   - Prism: Graceful recovery keeps method definitions with error flags
#
# Since this tests parser error recovery (not desugaring), these cases cannot
# be meaningfully compared between parsers.
# Tests for these cases are in test/testdata/parser/error_recovery/block_forwarding_invalid_def.rb.

# ==============================================================================
# VALID CASES - These should produce identical output
# ==============================================================================

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

# CASE 6: ... + { } at call site
# Forwarding includes <fwd-block>, AND there's a literal block.
# Both should be kept in the output.
def case6_forwarding_and_literal(...)
  foo(...) { "literal block" }
end

# CASE 7: & + { } at call site
# Block pass argument AND a literal block.
# Both should be kept in the output.
def case7_block_pass_and_literal(&block)
  foo(&block) { "literal block" }
end

# Case 8: Anonymous block param with literal block
def case8_anonymous_block_pass_and_literal(&)
  foo(&) { "literal block" }
end

# ==============================================================================
# ERROR CASES - These cannot be tested, see above
# ==============================================================================

# Case 9: & + ... in definition (named block param)
# def case9_block_param_and_forwarding(&block, ...)
#   bar(...)
# end

# Case 10: & + ... in definition (anonymous block param)
# def case10_anonymous_block_param_and_forwarding(&, ...)
#   bar(...)
# end

# Case 11: & + ... + { } (all three, named block param)
# def case11_all_three(&block, ...)
#   foo(...) { "literal block" }
# end

# Case 12: & + ... + { } (all three, anonymous block param)
# def case12_all_three_anonymous(&, ...)
#   foo(...) { "literal block" }
# end
