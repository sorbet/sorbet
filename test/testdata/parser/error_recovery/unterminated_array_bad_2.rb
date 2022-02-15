# typed: true

# We'd like a better parse for this case than we currently are getting. Specifically it would be nice if it didn't drop
# the 1 from the array, nor drop the "puts 'after'". If you can get closer to the desired behavior, please feel free to
# update the .exp file for this test.

class A
  def bar
    puts 'before'
    x = [1,
      # ^ error: unterminated "["
        # ^ error: unexpected token ","
    puts 'after'
  end
end
