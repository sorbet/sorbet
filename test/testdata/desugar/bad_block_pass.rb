# typed: true

def a(...); end

a[&blk] = 1
a.[]=(&blk, 1)
a(&blk, 1)
a.[]=(0, &blk, 1)
a(0, &blk, 1)
