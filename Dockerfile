FROM frolvlad/alpine-gxx

#RUN apt update && apt install gcc

#WORKDIR /app

COPY ./headers/* ./headers/
COPY ./impl/* ./impl/
COPY main.cpp ./

RUN g++ ./headers/judgeLetter.h ./headers/lexicalAnalysis.h ./headers/grammarAnalysis.h ./impl/judgeLetter.cpp ./impl/lexicalAnalysis.cpp ./impl/grammarAnalysis.cpp main.cpp -o main

RUN chmod +x main
