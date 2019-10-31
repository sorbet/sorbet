module Kernel
  alias sorbet_old_require require
  def require(name)
    if File.exists?(name)
      puts name
      IO.popen([__dir__ + '/compile', name])
    end
    sorbet_old_require(name)
  end
end
