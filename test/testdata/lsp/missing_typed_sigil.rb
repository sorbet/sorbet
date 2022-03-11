# !!!!
# THIS IS THE ONLY TEST FILE WITHOUT A TYPED SIGIL.
# Normally, we ban the use of test files lacking a sigil to ensure that all tests are intentionally typed at the
# expected level. However, this file tests a codepath that only triggers in files without a sigil, so it has been
# specially whitelisted.
# !!!!

# selective-apply-code-action: quickfix

class Hello
  def something(x)

  end
end

def main
  puts Helo.new
  #    ^^^^ error: Unable to resolve constant `Helo`
  #    ^^^^ apply-code-action: [A] Replace with `Hello`
end
