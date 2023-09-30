Installation steps for installing cryptopp library for linux:

    1) Download cryptopp860 from https://www.cryptopp.com/release860.html or use cryptopp860.zip provided.
    2) Extract crytopp860.zip. 
    3) Open terminal in the same folder and execute "make" command. 
    4) Execute "make test" for testing cryptopp library installation.
    5) Execute "make install" to install cryptopp library.

Steps to compile and executing the simulation:

    1) Open terminal in this repository.
    2) Execute "make" command. 
    3) Execute "./simulation" to start the simulation.

Terminology:
    Events: events refers to transactions that users are trying to generate.
    Requests: requests are at a miner level where they receive a event and try to work on the transaction.
    Block: it is a unit of blockchain where immutable transactions are stored.
    Genesis block: First block generate by the system.
    Latest block: refers to the latest block every miner uses to add new block.
    Miner: A member in the system that validates transactions and tries to add blocks to the blockchain.

Algorithm used:
    Proof of Elapsed Time (PoET): It is a network consensus algorithm used for low resource utilization.
    
Files:
    events.csv: contains events generated during the simulation.
    blockchain.csv: contains blocks generated during the simulation.
    request.csv: contains requests generated during the simulation.
