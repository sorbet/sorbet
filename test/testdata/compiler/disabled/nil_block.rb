# compiled: true
# typed: true
# frozen_string_literal: true

class A
  extend T::Sig
  extend T::Helpers

  def self.call(*args, &blk)
    begin
      T.unsafe(new).call(*args, &blk)
    rescue
      puts "whoops! #{$!}"
    end
  end
end

class B < A
  def call(name, &blk)
    yield "name = #{name}"
  end
end

B.call('Alice')
