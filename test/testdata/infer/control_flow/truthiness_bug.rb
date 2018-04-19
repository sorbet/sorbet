# typed: strict

class Test
  def set_approval_status(_, from_self)
    approved = _.foo?

    t1 = if approved then from_self else approved end

    if t1 then end

    # This used to report a dead-code error on the 'pending' below due
    # to a bug in CFG simplification
    puts(
      approved ? 'success' : 'pending'
    )
  end
end
