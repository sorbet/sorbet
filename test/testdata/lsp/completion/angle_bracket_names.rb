# typed: true

class A
  it 'this_is_a_test' do
  end

  def this_is_a_test
  end
end

A.new.this_is_a # error: does not exist
#              ^ completion: this_is_a_test

class B
  def default_arg(x: nil)
  end
end

B.new.default_ar # error: does not exist
#               ^ completion: default_arg
