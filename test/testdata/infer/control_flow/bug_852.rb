# typed: true
class Test
  def cond; end

  def test_case
    if cond && cond
      var = T.let(nil, T.nilable(String))
    end

    if @iv && var
      T.assert_type!(var, String)
    end
  end
end
