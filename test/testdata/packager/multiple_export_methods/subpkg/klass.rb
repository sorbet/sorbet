# typed: true

class Klass
  def method
    Main.a_method
    Main.b_method # error: Method `b_method` does not exist
    Main.c_method
  end
end
