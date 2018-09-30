# typed: true
class A
  extend T::Helpers

  sig {params(t: Time).returns(Time)}
  def foo(t)
    t.to_datetime.to_date.to_time
  end
end
