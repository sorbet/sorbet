# typed: true
class Main
  def foo
    [].flat_map(&:keys).map do |a|
      foo || bar(a)
    end
  end

  def bar(a)
  end
end
