FROM dockerregistry.local.corp.stripe.com/stripe/ubuntu-16.04:latest

RUN add-apt-repository ppa:openjdk-r/ppa && apt-get update && apt-get install -y openjdk-8-jdk zip
RUN update-java-alternatives -s java-1.8.0-openjdk-amd64
ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64/
ENV PATH $JAVA_HOME/bin:$PATH
RUN export JAVA_HOME

RUN apt-get update && apt-get install -y clang-4.0 clang-format-4.0

RUN /usr/stripe/bin/docker/stripe-install-go 1.9 d70eadefce8e160638a9a6db97f7192d8463069ab33138893ad3bf31b0650a79
ENV GOPATH /gopath
ENV PATH $PATH:/usr/local/go/bin:$GOPATH/bin
RUN mkdir -p "$GOPATH" && \
  go get -d -u github.com/bazelbuild/buildifier/buildifier && \
  go generate github.com/bazelbuild/buildifier/build && \
  go install github.com/bazelbuild/buildifier/buildifier

WORKDIR /src

ENV PATH $PATH:/src/

CMD /src/jenkins-build.sh
