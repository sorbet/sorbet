# typed: true
class Opus::CIBot::Tasks::NotifySlackBuildComplete
  extend T::Helpers

  def initialize()
    @determined_build_group_status_and_which = T.let(nil, T.nilable([T.any(TrueClass, FalseClass), T.nilable(String)]))
    nil
  end

  def cond; end;

  sig {returns([T.any(TrueClass, FalseClass), T.nilable(String)])}
  private def determined_build_group_status_and_which
    @determined_build_group_status_and_which ||= [true, "fail"]
  end
end
