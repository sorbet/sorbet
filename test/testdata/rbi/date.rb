# typed: strict
class A
  extend T::Helpers

  sig(t: Time).returns(Time)
  def foo(t)
    t.to_datetime.to_date.to_time
  end
end
