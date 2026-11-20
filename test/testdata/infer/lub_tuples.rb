# typed: true
class Opus::CIBot::Tasks::NotifySlackBuildComplete
  extend T::Sig

  def initialize()
    @determined_build_group_status_and_which = T.let(nil, T.nilable([T::Boolean, T.nilable(String)]))
    nil
  end

  def cond; end;

  sig {returns([T::Boolean, T.nilable(String)])}
  private def determined_build_group_status_and_which
    @determined_build_group_status_and_which ||= [true, "fail"]
  end
end

# Different lengths, same homogenous type
T.reveal_type(
  if T.unsafe(nil)
    [1, 2]
  else
    [1, 2, 3]
  end
) # error: Revealed type: `T::Array[Integer]`

# Different lengths, different types
T.reveal_type(
  if T.unsafe(nil)
    [1, "two"]
  else
    [1, 2, 3]
  end
) # error: Revealed type: `T::Array[T.untyped]`
