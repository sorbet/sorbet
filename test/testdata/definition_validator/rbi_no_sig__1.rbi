# typed: strict

def foo # error: The method `foo` does not have a `sig`
end

sig { void }
def has_sig_in_rbi
end

def has_sig_in_source
end
