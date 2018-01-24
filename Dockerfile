FROM 030465607062.dkr.ecr.us-west-2.amazonaws.com/stripe/build/ubuntu-16.04:latest

RUN add-apt-repository ppa:openjdk-r/ppa && apt-get update && apt-get install -y openjdk-8-jdk zip
RUN update-java-alternatives -s java-1.8.0-openjdk-amd64
ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64/
ENV PATH $JAVA_HOME/bin:$PATH
RUN export JAVA_HOME

RUN apt-get update && apt-get install -y clang-4.0 clang-format-4.0 time libncurses5-dev shellcheck

# Dependencies for the parser
RUN apt-get update && apt-get install -y ragel bison ruby autoconf

RUN /usr/stripe/bin/docker/stripe-install-go 1.9 d70eadefce8e160638a9a6db97f7192d8463069ab33138893ad3bf31b0650a79
ENV GOPATH /gopath
ENV PATH $PATH:/usr/local/go/bin:$GOPATH/bin
# `format_build_files.sh` will pin + install a specific version of
# buildifier, so don't install it here, just do the fetch to save a
# little time.
RUN mkdir -p "$GOPATH" && \
  go get -d -u github.com/bazelbuild/buildifier/buildifier && \
  go get -u github.com/stripe/veneur/cmd/veneur-emit

WORKDIR /src

ENV PATH $PATH:/src/

CMD /src/jenkins-build.sh
