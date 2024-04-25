# frozen_string_literal: true
# typed: true
# compiled: true

# Ruby blocks at the "toplevel" (i.e. no parent blocks) don't indicate their depth,
# whereas blocks with parent blocks ("deep" blocks below) do.

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
check do
  module X
    1.times do
      raise 'exception!'
    end
  end
end
check do
  module X
    1.times do
      1.times do
        raise 'exception!'
      end
    end
  end
end
check do
  class Y
    1.times do
      raise 'exception!'
    end
  end
end
check do
  class Y
    1.times do
      1.times do
        raise 'exception!'
      end
    end
  end
end
begin
  1.times do
    raise 'exception!'
 end
rescue => e
  p T.must(e.backtrace_locations).fetch(0).label
end

begin
  1.times do
    1.times do
      1.times do
        raise 'exception!'
      end
    end
  end
rescue => e
  p T.must(e.backtrace_locations).fetch(0).label
end

check do
  raise 'exception!'
end
check do
  1.times do
    1.times do
      raise 'exception!'
    end
  end
end
