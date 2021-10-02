FROM ubuntu

RUN sed -i 's#http://archive.ubuntu.com/#http://mirrors.tuna.tsinghua.edu.cn/#' /etc/apt/sources.list;
RUN apt update && apt install gcc

#WORKDIR /app

COPY headers impl main.cpp ./

RUN gcc main.cpp -o main

RUN chmod +x main