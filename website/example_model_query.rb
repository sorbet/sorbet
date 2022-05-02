# typed: strict

class Module; include T::Sig; end

class Scratch
  module IQuery
    extend T::Generic
    abstract!

    U = type_member(:out) {{upper: AbstractModel}}

    sig {abstract.returns(T::Array[U])}
    def provides; end
  end

  class Query
    extend T::Generic
    include IQuery

    U = type_member {{upper: AbstractModel}}

    sig {override.returns(T::Array[U])}
    def provides
      []
    end
  end

  class AbstractModel
    extend T::Helpers
    abstract!

    sig {abstract.returns(IQuery[AbstractModel])}
    def self.query; end
  end

  class ModelView
    extend T::Generic
    U = type_member {{upper: AbstractModel}}

    # -- old --
    # sig {params(type: T.class_of(U)).void}
    # def initialize(type)

    sig {void}
    def initialize
      @queries = T.let([], T::Array[IQuery[U]])
    end

    sig {params(q: IQuery[U]).returns(T.self_type)}
    def add_query(q)
      @queries << q
      self
    end

    sig {returns(T::Array[U])}
    def load_everything
      @queries.flat_map {|q| q.provides}
    end
  end

  class ModelAlice < AbstractModel
    sig {override.returns(IQuery[ModelAlice])}
    def self.query
      Query[ModelAlice].new
    end
  end

  class ModelBob < AbstractModel
    sig {override.returns(IQuery[ModelBob])}
    def self.query
      Query[ModelBob].new
    end
  end

  sig {void}
  def self.main
    ModelView[ModelAlice].new
      .add_query(ModelAlice.query)
      .add_query(ModelBob.query)
      .load_everything
  end
end
