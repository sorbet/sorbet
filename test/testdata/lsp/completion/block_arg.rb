# typed: true
extend T::Sig

def xy; end

10.times do |xyz|
  xyy = 1
  xy
  # ^ completion: xyy, xyz, xy
end

def method(xy_param)
  xy
  # ^ completion: xy_param, xy

  10.times do |xy_blk_param|
    xy
    # ^ completion: xy_blk_param, xy_param, xy
  end

  xy
  # ^ completion: xy_param, xy
end
