# typed: true

# Note that this test exists just for the purpose of validating lowering of zero-arg "super", via exp files. It's not
# expected to do anything useful if actually executed.
#
# For this reason, this test does double duty in checking that `--typed-super=false` works.
# typed-super: false

def pos(x, y, z)
  super
  super { puts "hi" }
end

def possplat(*foo)
  super
  super { puts "hi" }
end

def possplat_pos(*foo, p, q)
  super
  super { puts "hi" }
end

def pos_possplat(x, y, z, *foo)
  super
  super { puts "hi" }
end

def pos_possplat_pos(x, y, z, *foo, p, q)
  super
  super { puts "hi" }
end

def pos_blk(x, y, z, &blk)
  super
  super { puts "hi" }
end

def possplat_blk(*foo, &blk)
  super
  super { puts "hi" }
end

def possplat_pos_blk(*foo, p, q, &blk)
  super
  super { puts "hi" }
end

def pos_possplat_blk(x, y, z, *foo, &blk)
  super
  super { puts "hi" }
end

def pos_possplat_pos_blk(x, y, z, *foo, p, q, &blk)
  super
  super { puts "hi" }
end

def pos_kw(x, y, z, j:, k:)
  super
  super { puts "hi" }
end

def possplat_kw(*foo, j:, k:)
  super
  super { puts "hi" }
end

def possplat_pos_kw(*foo, p, q, j:, k:)
  super
  super { puts "hi" }
end

def pos_possplat_kw(x, y, z, *foo, j:, k:)
  super
  super { puts "hi" }
end

def pos_possplat_pos_kw(x, y, z, *foo, p, q, j:, k:)
  super
  super { puts "hi" }
end

def pos_kwsplat(x, y, z, **bar)
  super
  super { puts "hi" }
end

def possplat_kwsplat(*foo, **bar)
  super
  super { puts "hi" }
end

def possplat_pos_kwsplat(*foo, p, q, **bar)
  super
  super { puts "hi" }
end

def pos_possplat_kwsplat(x, y, z, *foo, **bar)
  super
  super { puts "hi" }
end

def pos_possplat_pos_kwsplat(x, y, z, *foo, p, q, **bar)
  super
  super { puts "hi" }
end

def pos_kw_kwsplat(x, y, z, j:, k:, **bar)
  super
  super { puts "hi" }
end

def possplat_kw_kwsplat(*foo, j:, k:, **bar)
  super
  super { puts "hi" }
end

def possplat_pos_kw_kwsplat(*foo, p, q, j:, k:, **bar)
  super
  super { puts "hi" }
end

def pos_possplat_kw_kwsplat(x, y, z, *foo, j:, k:, **bar)
  super
  super { puts "hi" }
end

def pos_possplat_pos_kw_kwsplat(x, y, z, *foo, p, q, j:, k:, **bar)
  super
  super { puts "hi" }
end

def pos_kw_blk(x, y, z, j:, k:, &blk)
  super
  super { puts "hi" }
end

def possplat_kw_blk(*foo, j:, k:, &blk)
  super
  super { puts "hi" }
end

def possplat_pos_kw_blk(*foo, p, q, j:, k:, &blk)
  super
  super { puts "hi" }
end

def pos_possplat_kw_blk(x, y, z, *foo, j:, k:, &blk)
  super
  super { puts "hi" }
end

def pos_possplat_pos_kw_blk(x, y, z, *foo, p, q, j:, k:, &blk)
  super
  super { puts "hi" }
end

def pos_kwsplat_blk(x, y, z, **bar, &blk)
  super
  super { puts "hi" }
end

def possplat_kwsplat_blk(*foo, **bar, &blk)
  super
  super { puts "hi" }
end

def possplat_pos_kwsplat_blk(*foo, p, q, **bar, &blk)
  super
  super { puts "hi" }
end

def pos_possplat_kwsplat_blk(x, y, z, *foo, **bar, &blk)
  super
  super { puts "hi" }
end

def pos_possplat_pos_kwsplat_blk(x, y, z, *foo, p, q, **bar, &blk)
  super
  super { puts "hi" }
end

def pos_kw_kwsplat_blk(x, y, z, j:, k:, **bar, &blk)
  super
  super { puts "hi" }
end

def possplat_kw_kwsplat_blk(*foo, j:, k:, **bar, &blk)
  super
  super { puts "hi" }
end

def possplat_pos_kw_kwsplat_blk(*foo, p, q, j:, k:, **bar, &blk)
  super
  super { puts "hi" }
end

def pos_possplat_kw_kwsplat_blk(x, y, z, *foo, j:, k:, **bar, &blk)
  super
  super { puts "hi" }
end

def pos_possplat_pos_kw_kwsplat_blk(x, y, z, *foo, p, q, j:, k:, **bar, &blk)
  super
  super { puts "hi" }
end
