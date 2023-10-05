FROM quay.io/pypa/manylinux_2_28_x86_64
ADD . /src
RUN mkdir /output
WORKDIR /src
ENTRYPOINT [ "bash", "scripts/build--docker.sh" ]

