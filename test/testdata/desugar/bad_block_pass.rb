# typed: true

def a(...); end

a[&blk] = 1
# ^^^^ error: Unsupported block pass node in non-final argument
a.[]=(&blk, 1)
#     ^^^^ error: Unsupported block pass node in non-final argument
#         ^ error: Unexpected token ","
a(&blk, 1)
# ^^^^ error: Unsupported block pass node in non-final argument
a.[]=(0, &blk, 1)
#        ^^^^ error: Unsupported block pass node in non-final argument
#            ^ error: Unexpected token ","
# ^^^^ error: Unsupported block pass node in non-final argument
a(0, &blk, 1)
#    ^^^^ error: Unsupported block pass node in non-final argument
#        ^ error: Unexpected token ","
