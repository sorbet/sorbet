# typed: true

class ComputingProps
  extend T::Sig

  const :num_ok, Integer, computed_by: :compute_num_ok
  sig {params(n: Integer).returns(Integer)}
  def self.compute_num_ok(n)
    10
  end

  const :missing, Integer, computed_by: :compute_missing
                                      # ^^^^^^^^^^^^^^^^ error: Method `compute_missing` does not exist on `T.class_of(ComputingProps)`

  const :num_wrong_value, Integer, computed_by: :compute_num_wrong_value
                                              # ^^^^^^^^^^^^^^^^^^^^^^^^ error: Returning value that does not conform to method result type
  sig {params(inputs: T.untyped).returns(String)}
  def self.compute_num_wrong_value(inputs)
    'not_an_integer'
  end

  const :num_wrong_type, Integer, computed_by: :compute_num_wrong_type
  sig {params(inputs: T.untyped).returns(Integer)}
  def self.compute_num_wrong_type(inputs)
    'not_an_integer' # error: Returning value that does not conform to method result type
  end

  const :not_a_symbol, String, computed_by: 'not_a_symbol'
                                          # ^^^^^^^^^^^^^^ error: Argument does not have asserted type `Symbol`

  const :num_unknown_type, Integer, computed_by: :compute_num_unknown_type
                                               # ^^^^^^^^^^^^^^^^^^^^^^^^ error: Returning value that does not conform to method result type
  def self.compute_num_unknown_type(inputs)
    T.untyped(nil)
  end
end
