# @typed
class MyClass
  def foo=(arg)
  end
end

def foo
    begin
        raise "boop"
    rescue Exception => MyClass.new.foo # error: Not enough arguments provided for method foo=
        3
    end
end
puts foo
