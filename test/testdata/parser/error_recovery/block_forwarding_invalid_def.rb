# typed: false
# disable-parser-comparison: true

# Cases where `&` and `...` appear together in a method definition.
# This is invalid Ruby syntax - you cannot have both a block param and forwarding.
# These are error recovery tests for cases 9-12 which could not be tested in
# test/prism_regression/call_forwarding_permutations.rb. Cases 1-8 are tested in that file.
#
# Cases:
#    9. & + ...                   - def f(&b, ...); bar(...); end
#   10. & (anonymous) + ...       - def f(&, ...); bar(...); end
#   11. & + ... + { }             - def f(&b, ...); bar(...) { }; end
#   12. & (anonymous) + ... + { } - def f(&, ...); bar(...) { }; end

# Case 9: & + ...
def case9_block_param_and_forwarding(&block, ...)
#                                          ^ error: unexpected token ","
  bar(...)
#     ^^^ error: unexpected token "..."
end # error: unexpected token "end"

# Case 10: & (anonymous) + ...
def case10_anonymous_block_param_and_forwarding(&, ...)
#                                                ^ error: unexpected token ","
  bar(...)
#     ^^^ error: unexpected token "..."
end # error: unexpected token "end"

# Case 11: & + ... + { }
def case11_all_three(&block, ...)
#                          ^ error: unexpected token ","
  foo(...) { "literal block" }
#     ^^^ error: unexpected token "..."
end

# Case 12: & (anonymous) + ... + { }
def case12_all_three_anonymous(&, ...)
#                               ^ error: unexpected token ","
  foo(...) { "literal block" }
#     ^^^ error: unexpected token "..."
end
