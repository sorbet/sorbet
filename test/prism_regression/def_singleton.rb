# typed: false

def self.foo; end
def x.foo; end # This is invalid

def self.foo(&blk)
  super(&blk)
end
