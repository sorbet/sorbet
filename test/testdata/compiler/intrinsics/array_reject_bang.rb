# frozen_string_literal: true
# typed: true
# compiled: true

def test_blk
  arr = [1, 2, 3, 4, 5]
  arr.reject! { |x| x%2 == 0 }

  p arr
  p arr.length
end

test_blk


def test_blk_raise
  arr = [1, 2, 3, 4, 5]
  begin
    arr.reject! do |x|
      if x > 3
        raise "whoops"
      end
      x%2 == 0
    end
  rescue => e
    puts e
  end

  p arr
  p arr.length
end

test_blk_raise


def test_blk_break
  arr = [1, 2, 3, 4, 5]
  arr.reject! do |x|
    if x > 3
      break
    end
    x%2 == 0
  end

  p arr
  p arr.length
end

test_blk_break


def test_no_blk
  arr = [1, 2, 3, 4, 5]
  e = arr.reject!

  p arr
  p arr.length

  e.each { |x| x%2 == 0 }

  p arr
  p arr.length
end

test_no_blk

