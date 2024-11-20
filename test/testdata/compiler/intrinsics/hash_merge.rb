# frozen_string_literal: true
# typed: true
# compiled: true

def no_block
  h1 = {x: "foo", q: "bar", z: 33}

  p (h1.merge({x: "bar", z: 33, j: 96}))

  p h1
end


def with_block
  h1 = {x: "foo", q: "bar", z: 33}

  p (h1.merge({x: "bar", z: 33, j: 96}) do |key, oldval, newval|
       "key: #{key}, oldval: #{oldval}, newval: #{oldval}"
     end)

  p h1
end


with_block

def with_block_raise
  h1 = {x: "foo", q: "bar", z: 33}

  begin
    h1.merge({x: "bar", z: 33, j: 96}) do |key, oldval, newval|
      if key == :z
        raise "boom"
      end
      newval
    end
  rescue => e
    puts e
  end

  p h1
end


with_block_raise