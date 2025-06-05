# typed: true

class A
  def value
    5
  end
end

extend T::Sig

sig { params(as: A).returns(T.untyped) }
def sumup_splat(*as)
  sumup(as)
end

sig { params(avec: T::Array[A], blk: T.proc.params(a0: A).returns(T.untyped)).returns(T.untyped) }
def call_with_block(avec, &blk)
  avec.su(&blk) # error: does not exist
#        ^ completion: sum
end

sig { params(as: A, blk: T.proc.params(a0: A).returns(T.untyped)).returns(T.untyped) }
def sumup_splat_with_block(*as, &blk)
  as.su(&blk) # error: does not exist
#      ^ completion: sum
end

sig { params(avec: T::Array[A]).returns(T.untyped) }
def sumup(avec)
end

def f
  as = [A.new]
  sumup_(*as) # error: does not exist
#       ^ completion: sumup_splat, sumup_splat_with_block
end

def g(&blk)
  as = [A.new]
  sumup_(*as, &blk) # error: does not exist
#       ^ completion: sumup_splat, sumup_splat_with_block
end
