FROM gcc:10.2

#RUN apt update && apt install gcc

#WORKDIR /app

COPY ./headers/* ./headers/
COPY ./impl/* ./impl/
COPY main.cpp ./

RUN g++ ./headers/* ./impl/* main.cpp -o main

RUN chmod +x main
