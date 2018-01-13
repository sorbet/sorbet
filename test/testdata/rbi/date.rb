# @typed
class A
  sig(t: Time).returns(Time)
  def foo(t)
    t.to_datetime.to_date.to_time
  end
end
