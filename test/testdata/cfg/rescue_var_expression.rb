# @typed
class MyClass
  def foo=(arg)
  end
end

def foo
    begin
        raise "boop"
    rescue Exception => MyClass.new.foo
        3
    end
end
puts foo
