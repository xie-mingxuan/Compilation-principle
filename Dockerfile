FROM gcc

RUN sed -i 's#http://archive.ubuntu.com/#http://mirrors.tuna.tsinghua.edu.cn/#' /etc/apt/sources.list;
RUN apt update && apt install gcc

#WORKDIR /app

COPY headers impl main.cpp ./

RUN g++ -c main.cpp -o main.o
RUN g++ main.o -o main -lstdc++

RUN chmod +x main