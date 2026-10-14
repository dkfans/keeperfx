FROM debian:trixie
RUN apt-get update -qq \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y -qq gcc g++ zlib1g-dev curl ca-certificates \
        cppcheck clang-tidy valgrind python3 \
    && rm -rf /var/lib/apt/lists/*
