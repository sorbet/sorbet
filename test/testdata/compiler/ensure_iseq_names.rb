# frozen_string_literal: true
# typed: true
# compiled: true

class A
  def self.class_func_toplevel_rescue
    begin
      raise 'start'
    ensure
      raise 'real exception'
    end
  end

  def instance_func_toplevel_rescue
    begin
      raise 'start'
    ensure
      raise 'real exception'
    end
  end
end

def standalone_func_toplevel_rescue
  begin
    raise 'start'
  ensure
    raise 'real exception'
  end
end

def check(&blk)
  begin
    yield
  rescue => e
    p T.must(e.backtrace_locations).fetch(0).label
  end
end

check {A.class_func_toplevel_rescue}
check {A.new.instance_func_toplevel_rescue}
check {standalone_func_toplevel_rescue}
check do
  module X
    begin
      raise 'start'
    ensure
      raise 'real exception'
    end
  end
end
check do
  class Y
    begin
      raise 'start'
    ensure
      raise 'real exception'
    end
  end
end
