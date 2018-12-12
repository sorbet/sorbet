# typed: true
class A
  extend T::Sig

  sig {params(t: Time).returns(Time)}
  def foo(t)
    t.to_datetime.to_date.to_time
  end
end
