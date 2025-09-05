# typed: true

def before; end

class A
  def foo
    xyz = "
    1.times do
      xyz = nil
    end
  end
end

def after
end
