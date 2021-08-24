# frozen_string_literal: true
# typed: true
# compiled: true

class A
  def self.class_func_single_block_in_rescue
    begin
      raise 'start'
    rescue
      1.times do
        raise 'real exception'
      end
    end
  end

  def self.class_func_nested_block_in_rescue
    begin
      raise 'start'
    rescue
      1.times do
        1.times do
          1.times do
            raise 'real exception'
          end
        end
      end
    end
  end

  def self.class_func_single_block_in_rescue_in_block
    1.times do
      begin
        raise 'start'
      rescue
        1.times do
          raise 'real exception'
        end
      end
    end
  end

  def instance_func_single_block_in_ensure
    begin
      raise 'start'
    ensure
      1.times do
        raise 'real_exception'
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

check {A.class_func_single_block_in_rescue}
check {A.class_func_nested_block_in_rescue}
check {A.class_func_single_block_in_rescue_in_block}
check {A.new.instance_func_single_block_in_ensure}
