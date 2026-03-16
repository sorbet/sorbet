# typed: true

  self[&:to_s] &= 123456
#             | error: Method `[]` does not exist
# ^^^^^^^^^^^^ error: Setter method `[]=` does not exist

class A
  def [](*args, **kwargs, &blk)
    p [args, kwargs, blk]
    0
  end
  def []=(*args, **kwargs, &blk)
    p [args, kwargs, blk]
    0
  end
end

p(A.new[x: 2] += 3)
