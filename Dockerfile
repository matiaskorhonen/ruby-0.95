FROM i386/debian:jessie

# Set the working directory to /app
WORKDIR /app

# Copy the current directory contents into the container at /app
COPY . /app

RUN mkdir -p /ruby95/bin
RUN mkdir -p /ruby95/lib/ruby

RUN apt-get update -yqq && apt-get install -yqq --no-install-recommends autoconf bison build-essential flex
RUN ./configure --host=i386-unknown-linux --prefix="/ruby95"
RUN make
RUN make install
ENTRYPOINT ["/ruby95/bin/ruby"]
