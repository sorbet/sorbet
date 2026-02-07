# typed: false

# This file tests combinations of block forwarding and literal blocks.
#
# Summary of 8 combinations:
#   Valid:
#     1. None           - def f; bar; end
#     2. Just &         - def f(&b); bar(&b); end
#     3. Just ...       - def f(...); bar(...); end
#     4. Just { }       - def f; bar { }; end
#     5. ... + { }      - def f(...); bar(...) { }; end
#     6. & + { }        - def f(&b); bar(&b) { }; end
#   Error cases (definition - cannot be tested, see below):
#     7. & + ...        - def f(&, ...); end
#     8. & + ... + { }  - def f(&, ...); bar(...) { }; end

# Cases 7, 7a, 8, 8a are excluded from this test.
#
# These cases have `&` and `...` together in the definition, which is a parser
# error. The error recovery is fundamentally different between parsers:
#   - Whitequark: Aggressive recovery destroys the entire file from error point
#   - Prism: Graceful recovery keeps method definitions with error flags
#
# Since this tests parser error recovery (not desugaring), these cases cannot
# be meaningfully compared between parsers.

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

# Case 2a: Anonymous block parameter
def case2a_anonymous_block_param(&)
  bar(&)
end

# Case 3: Just ... - Forwarding only
def case3_forwarding(...)
  bar(...)
end

# Case 4: Just { } - Block literal only
def case4_block_literal
  bar { "block body" }
end

# CASE 5: ... + { } at call site
# Forwarding includes <fwd-block>, AND there's a literal block.
# Both should be kept in the output.
def case5_forwarding_and_literal(...)
  foo(...) { "literal block" }
end

# CASE 6: & + { } at call site
# Block pass argument AND a literal block.
# Both should be kept in the output.
def case6_block_pass_and_literal(&block)
  foo(&block) { "literal block" }
end

# Case 6a: Anonymous block param with literal block
def case6a_anonymous_block_pass_and_literal(&)
  foo(&) { "literal block" }
end

# ==============================================================================
# ERROR CASES - These cannot be tested, see above
# ==============================================================================

# Case 7: & + ... in definition (named block param)
# def case7_block_param_and_forwarding(&block, ...)
#   bar(...)
# end

# Case 7a: & + ... in definition (anonymous block param)
# def case7a_anonymous_block_param_and_forwarding(&, ...)
#   bar(...)
# end

# Case 8: & + ... + { } (all three, named block param)
# def case8_all_three(&block, ...)
#   foo(...) { "literal block" }
# end

# Case 8a: & + ... + { } (all three, anonymous block param)
# def case8a_all_three_anonymous(&, ...)
#   foo(...) { "literal block" }
# end
