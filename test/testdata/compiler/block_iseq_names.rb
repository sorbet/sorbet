# frozen_string_literal: true
# typed: true
# compiled: true

class A
  def self.class_func_toplevel_block
    1.times do
      raise 'exception!'
    end
  end

  def self.class_func_deep_block
    1.times do
      1.times do
        1.times do
          raise 'exception!'
        end
      end
    end
  end

  def instance_func_toplevel_block
    1.times do
      raise 'exception!'
    end
  end

  def instance_func_deep_block
    1.times do
      1.times do
        1.times do
          raise 'exception!'
        end
      end
    end
  end
end

def standalone_func_toplevel_block
  1.times do
    raise 'exception!'
  end
end

def standalone_func_deep_block
  1.times do
    1.times do
      1.times do
        raise 'exception!'
      end
    end
  end
end

def check(&blk)
  begin
    yield
  rescue => e
    p T.must(e.backtrace_locations).fetch(0).label
  end
end

check {A.class_func_toplevel_block}
check {A.class_func_deep_block}
check {A.new.instance_func_toplevel_block}
check {A.new.instance_func_deep_block}
check {standalone_func_toplevel_block}
check {standalone_func_deep_block}
