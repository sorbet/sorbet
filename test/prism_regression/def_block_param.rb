# typed: false

def foo(&blk); end

def foo(&); end

def foo(&blk)
  super(&blk)
end
