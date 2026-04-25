FROM fedora:43

ARG ZULU_URL="https://cdn.azul.com/zulu/bin/zulu17.62.17-ca-jdk17.0.17-linux_x64.tar.gz"
ARG ZULU_DIR="/opt/zulu"

RUN dnf -y upgrade --refresh && \
    dnf -y install \
      maven \
      git \
      vim-enhanced \
      tree \
      less \
      which \
      findutils \
      procps-ng \
      tar \
      gzip \
      unzip \
      curl && \
    dnf clean all && \
    rm -rf /var/cache/dnf

RUN mkdir -p /opt && \
    curl -fL "${ZULU_URL}" -o /tmp/zulu.tar.gz && \
    tar -xzf /tmp/zulu.tar.gz -C /opt && \
    extracted_dir="$(tar -tzf /tmp/zulu.tar.gz | head -1 | cut -d/ -f1)" && \
    mv "/opt/${extracted_dir}" "${ZULU_DIR}" && \
    rm -f /tmp/zulu.tar.gz

ENV JAVA_HOME=/opt/zulu
ENV PATH=$JAVA_HOME/bin:$PATH
ENV MAVEN_OPTS="-Dmaven.repo.local=/root/.m2/repository"

WORKDIR /workspace
CMD ["sleep", "infinity"]
