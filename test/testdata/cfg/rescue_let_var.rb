# typed: true
# frozen_string_literal: true

extend T::Sig

def mismatched_but_for_effect(params)
  error_messages = T.let([], T::Array[String])

  begin
    T.let(params[:contents], T::Hash[String, String])
  rescue TypeError
    error_messages << "contents must be a hash of {path : contents}"
  end
  nil
end

def mismatched_return_value(params)
  error_messages = T.let([], T::Array[String])

  begin
    T.let(params[:contents], T::Hash[String, String])
  rescue TypeError
    error_messages << "contents must be a hash of {path : contents}" # error: Incompatible assignment to variable declared via `let`: `T::Array[String]` is not a subtype of `T::Hash[String, String]`
  end
end

error_messages = T.let([], T::Array[String])
params = { contents: "thing" }

begin
  T.let(params[:contents], T::Hash[String, String])
rescue TypeError
  error_messages << "contents must be a hash of {path : contents}"
end
