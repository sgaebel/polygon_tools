FROM quay.io/pypa/manylinux_2_28_x86_64
ADD . /src
RUN mkdir /output
WORKDIR /src
ENTRYPOINT [ "bash", "scripts/build_script.sh" ]

# python3.10 -m build --outdir /output
# python3.10 -m build --sdist --outdir /output
# docker container ls
# docker cp <container_name>:/output dist
