# typed: true
class Module; include T::Sig; end

module A; end
module B; end
module C; end
module D; end

module YieldsA
  sig { params(blk: T.proc.params(arg0: A).void).void }
  def go(&blk)
  end
end

module YieldsB
  sig { params(blk: T.proc.params(arg0: B).void).void }
  def go(&blk)
  end
end

module YieldsC
  sig { params(blk: T.proc.params(arg0: C).void).void }
  def go(&blk)
  end
end

module YieldsD
  sig { params(blk: T.proc.params(arg0: D).void).void }
  def go(&blk)
  end
end

sig { params(x: T.all(T.any(YieldsA, YieldsB), T.any(YieldsC, YieldsD))).void }
def example1(x)
  x.go do |arg0|
    T.reveal_type(arg0)
  end
end

sig { params(x: T.any(YieldsA, YieldsB)).void }
def example2(x)
  x.go do |arg0|
    T.reveal_type(arg0)
  end
end

sig { params(x: T.all(T.any(YieldsA, YieldsB), YieldsC)).void }
def example3(x)
  x.go do |arg0|
    T.reveal_type(arg0)
  end
end

sig { params(x: T.all(YieldsA, T.any(YieldsB, YieldsC))).void }
def example4(x)
  x.go do |arg0|
    T.reveal_type(arg0)
  end
end

sig { params(x: T.any(YieldsA, T.all(YieldsB, YieldsC))).void }
def example5(x)
  x.go do |arg0|
    T.reveal_type(arg0)
  end
end

sig { params(x: T.any(T.all(YieldsA, YieldsB), T.all(YieldsC, YieldsD))).void }
def example6(x)
  x.go do |arg0|
    T.reveal_type(arg0)
  end
end

