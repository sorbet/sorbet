# typed: true

class A
  private def my_private_method; end

  def example
    my_private_metho
  end
end

A.new.my_private_metho
