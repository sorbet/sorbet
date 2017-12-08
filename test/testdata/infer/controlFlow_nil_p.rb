# @typed
class A
  standard_method({s: Opus::Types.nilable(String)}, returns: NilClass)
  def test_return(s)
    if s.nil?
      return
    end

    s.length
    nil
  end
end
