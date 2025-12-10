# typed: false
# disable-parser-comparison: true

# Cases where `&` and `...` appear together in a method definition.
# This is invalid Ruby syntax - you cannot have both a block param and forwarding.
#
# Cases:
#   7. & + ... (named)  - def f(&b, ...); bar(...); end
#   7a. & + ... (anon)  - def f(&, ...); bar(...); end
#   8. & + ... + { } (named) - def f(&b, ...); bar(...) { }; end
#   8a. & + ... + { } (anon) - def f(&, ...); bar(...) { }; end

# Case 7: & + ... (named)
def case7_block_param_and_forwarding(&block, ...)
#                                          ^ error: unexpected token ","
  bar(...)
#     ^^^ error: unexpected token "..."
end # error: unexpected token "end"

# Case 7a: & + ... (anonymous)
def case7a_anonymous_block_param_and_forwarding(&, ...)
#                                                ^ error: unexpected token ","
  bar(...)
#     ^^^ error: unexpected token "..."
end # error: unexpected token "end"

# Case 8: & + ... + { } (named)
def case8_all_three(&block, ...)
#                         ^ error: unexpected token ","
  foo(...) { "literal block" }
#     ^^^ error: unexpected token "..."
end

# Case 8a: & + ... + { } (anonymous)
def case8a_all_three_anonymous(&, ...)
#                               ^ error: unexpected token ","
  foo(...) { "literal block" }
#     ^^^ error: unexpected token "..."
end
