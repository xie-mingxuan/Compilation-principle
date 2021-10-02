FROM ubuntu

RUN apt update && apt install gcc

#WORKDIR /app

COPY headers impl main.cpp ./

RUN gcc main.cpp -o main

RUN chmod +x main