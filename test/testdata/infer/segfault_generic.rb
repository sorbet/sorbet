# typed: true

def bar
end

def foo
  loop do
    if bar
      type = T::Array[Integer]
    elsif bar
      puts type
    end

    begin
    rescue
    end
  end
end
