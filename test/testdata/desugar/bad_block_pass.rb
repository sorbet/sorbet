# typed: true

def a(...); end

a[&blk] = 1
# ^^^^ error: Unsupported block pass node in non-final argument
a.[]=(&blk, 1)
#     ^^^^ error: Unsupported block pass node in non-final argument
#         ^ error: unexpected token ","
a(&blk, 1)
# ^^^^ error: Unsupported block pass node in non-final argument
#     ^ error: unexpected token ","
a.[]=(0, &blk, 1)
#        ^^^^ error: Unsupported block pass node in non-final argument
#            ^ error: unexpected token ","
a(0, &blk, 1)
#    ^^^^ error: Unsupported block pass node in non-final argument
#        ^ error: unexpected token ","
