# typed: strict
class A
  class Foo
    sig.returns(T.any(String, NilClass))
    def name;end
  end

  sig(repo: String, branch: String).returns(T::Array[T.nilable(String)])
  def automatic_job_names(repo, branch)
    job_names = T::Array[Foo].new.map(&:name)
    job_names.select do |job_name|
     job_name
    end
  end
end
