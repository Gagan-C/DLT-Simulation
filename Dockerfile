FROM ubuntu

WORKDIR /simulation

COPY cryptopp860.zip .

RUN apt-get update
RUN apt-get install -y zip \g++ \make

RUN unzip cryptopp860.zip -d ./cryptopp860
RUN cd cryptopp860 && make
RUN cd cryptopp860 && make test
RUN cd cryptopp860 && make install

COPY main.cpp .
COPY makefile .

RUN make 

