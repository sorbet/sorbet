class Destructure
  def f((x,y), z)
    x + y

    lambda do |(a,b)| # error: lambda does not exist
    end
  end
end
