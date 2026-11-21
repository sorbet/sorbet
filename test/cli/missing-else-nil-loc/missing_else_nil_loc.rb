# typed: true
extend T::Sig

sig {params(x: Integer).void}
def takes_integer(x); end

sig {params(c: T::Boolean, x: Integer).void}
def example(c, x)
  takes_integer(if c then 1 end)

  takes_integer(
    if x == 1
      1
    elsif x == 2
      2
    end
  )

  takes_integer(
    case x
    when 1 then 1
    when 2 then 2
    end
  )

  takes_integer(
    case x
    when 1
    when 2 then 2
    else 3
    end
  )

  takes_integer(
    case x
    in 1 then 1
    in 2 then 2
    end
  )

  takes_integer(
    case x
    in 1
    in 2 then 2
    else 3
    end
  )

  takes_integer(unless c then 1 end)

  takes_integer((1 if c))
end
