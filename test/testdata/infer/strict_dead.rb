# @typed

module Main
  def foo
  end

  def main
    foo do |(k, v), h|
      h[k] = case v
      when Integer, NilClass # we used to have a false unreachable message here
        v
      end
    end
  end
end

