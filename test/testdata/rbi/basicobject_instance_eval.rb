# typed: true

T.reveal_type(4.instance_eval {4}) # error: Revealed type: `Integer`
4.instance_eval {T.reveal_type(self)} # error: Revealed type: `T.untyped`
4.instance_eval {|x| T.reveal_type(x)} # error: Revealed type: `T.untyped`
