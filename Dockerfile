FROM frolvlad/alpine-gxx

#RUN apt update && apt install gcc

#WORKDIR /app

COPY ./headers ./impl main.cpp ./

RUN g++ ./headers/judgeLetter.h ./headers/lexicalAnalysis.h ./impl/judgeLetter.cpp ./impl/lexicalAnalysis.cpp main.cpp -o main

RUN chmod +x main