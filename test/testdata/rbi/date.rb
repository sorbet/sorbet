# @typed
class A
  standard_method({t: Time}, returns: Time)
  def foo(t)
    t.to_datetime.to_date.to_time
  end
end
