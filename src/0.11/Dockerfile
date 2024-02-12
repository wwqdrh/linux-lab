FROM ubuntu:latest

RUN apt-get update && apt-get install -y wget gcc-multilib bin86 make bear

RUN wget http://old-releases.ubuntu.com/ubuntu/pool/main/g/gcc-3.4/cpp-3.4_3.4.6-6ubuntu2_amd64.deb \
    && wget http://old-releases.ubuntu.com/ubuntu/pool/main/g/gcc-3.4/gcc-3.4-base_3.4.6-6ubuntu2_amd64.deb \
    && wget http://old-releases.ubuntu.com/ubuntu/pool/main/g/gcc-3.4/gcc-3.4_3.4.6-6ubuntu2_amd64.deb

RUN dpkg -i gcc-3.4-base_3.4.6-6ubuntu2_amd64.deb

RUN dpkg -i cpp-3.4_3.4.6-6ubuntu2_amd64.deb

RUN dpkg -i gcc-3.4_3.4.6-6ubuntu2_amd64.deb
