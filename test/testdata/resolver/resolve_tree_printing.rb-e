# typed: true
class A
  def has_while
    while 1
      has_while
    end
  end

  def has_constant_with_resolution_scope
    DOES_NOT_EXIST # error: Unable to resolve constant
  end

  def has_global_field
    $S
  end

  def has_class_field
    @@f
  end
  
  def has_next
    loop do
      next
    end
  end

  def has_break
    loop do
      break
    end
  end

  def has_return
    return 1
  end

  def has_cast
    T.cast(nil, T.nilable(Integer))
  end
end
