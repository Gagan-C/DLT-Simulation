#include <iostream>
#include <string>
#include<queue>
#include <fstream>
#include<vector>
#include <string>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>

using namespace std;

/* Transaction type */
enum transactionType {T0, T1, T2, T3};


int INIT_EVENT_QUEUE_SIZE= 200;
int eventCount=1;
int tick=0;
const int MAX_TRANSACTION_COUNT=6;
const int MAX_EVENT_COUNT=3000;
const string MINER_E="E";
const int ALWAYS_ACTIVE_CUSTOMER_COUNT=4;
int MAX_CUSTOMER_COUNT=6;
int blockCounter=0;
int MAX_TEMPORARY_CUSTOMER=10;
int MAX_TEMPORARY_MINER=15;
int ALWAYS_ACTIVE_MINER_COUNT=1;


ofstream eventFile,requestFile,blockFile;
struct candidateblock;
struct event;

struct customer{
    int id;
    int dollars;
    std::string name;
    bool isActive;
};

struct transaction{
     int transactionId;
    transactionType type;
    int tick;
    customer sender;
    customer reciever;
    double amount;
};

struct block{
    int blockId;
    transaction trans[MAX_TRANSACTION_COUNT];
    std::string minedBy;
    std::string prevHash;
    block * prev=nullptr;
}genesisBlock,*latestBlock;



/*Prototypes*/
void openAllFiles();
void closeAllFiles();
void writeTransactionEventsToFile(transaction);
void writeCustomerEventsToFile(event);
void writeMinerEventsToFile(event);
void writeRequestsToFile(transaction,string);
void writeBlocktoFile(block);
void readAndWriteToEventFile(queue<transaction> , ofstream &);
string printTransType(transactionType);
customer getActiveCustomer();


/* Miner class */
class miner
{
    private:
        // vector<block> candidateBlock;
        block tentativeBlock;
        vector <candidateblock> candidateBlocks;
        int size=0;
        std::string name;
        // block *latestBlock;
        int waitTicks;
        int minedBlock=0;
        int sucessAttemptTransactions=0;
        std::string hashstring(std::string);
        std::string hashBlock();
        void updateCustomer( transaction);
    public:
        bool isActive;
        miner(string);
        miner();
        bool validateTransaction(transaction );
        void addTransactionToTentativeBlock(transaction);
        bool isReady();
        void flipStatus();
        int getSize();
        std::string getName();
        void addBlock(block);
        void flush(transaction []);
        int getCountOfAttemptTransactions();
        int getCountOfMinnedBlocks();
        transaction * getTransaction();
        void setName(string);
        // void refreshLatestBlock(block *);
        ~miner();
};


struct event{
    int tick;
    transaction tran;
    bool isTransaction;
    bool isCustomer;
    bool isMiner;
    bool addOrRemove;   /*Add true and Remove false*/
    customer cust;
    miner *m;
};
struct candidateblock{
    block b;
    int transactionCount=0;
};
vector <customer> cust;
vector <miner> miners;
queue <event> eventQueue;

/*more prototype*/
miner * getActiveMiner();
event generateSingleEvent();
customer getRandomCustomerToFlipStatus();
miner * getRandomMinerToFlipStatus();
void generateEventQueue(int);
void triggerEvent();
void initMiners();
void initCustomer();


void initMiners(){
    miner m(MINER_E);
    miners.push_back(m);
    for (int i = 0; i < MAX_TEMPORARY_MINER; i++)
    {
        miner temporaryMiner;
        temporaryMiner.setName("Miner "+to_string(miners.size()));
        miners.push_back(temporaryMiner);
    }
}


int main() {

    openAllFiles();
    initMiners();
    initCustomer();
    srand(time(NULL));
    latestBlock=&genesisBlock;
    generateEventQueue(INIT_EVENT_QUEUE_SIZE);
    int counter=0;
    while((!eventQueue.empty()) && eventQueue.size()<=MAX_EVENT_COUNT){
            
            // cout<<eventQueue.front().isTransaction <<" "<<eventQueue.front().isMiner<<" "<<eventQueue.front().isCustomer<<
            // "  "<<eventQueue.front().m.getName()<<" "<<eventQueue.front().cust.name<<" "<<eventQueue.size()<<" "<<
            // eventQueue.front().tran.sender.name<<" "<<eventQueue.front().tran.amount
            // <<endl;
            //generate random number of events when processing
        triggerEvent();
        counter++;
        
    }

     cout<<"Average number of transaction attempts per tick: ";
    double averageAttempts=0;
    double averageBlocks=0;
    for(long unsigned int i=0;i<miners.size();i++){
        averageAttempts=averageAttempts+miners[i].getCountOfAttemptTransactions();
        averageBlocks=miners[0].getCountOfMinnedBlocks();
    }
    averageAttempts=averageAttempts/(double)(tick);
    cout<<averageAttempts<<endl;
    cout<<"Average number of Blocks mined per miner per tick: ";
    averageBlocks=averageBlocks/(double)(tick);
    cout<<averageBlocks<<endl;
    closeAllFiles();
    return 0;
}

/* Miner defination*/
void miner::flipStatus(){
    this->isActive=!(this->isActive);
}
string miner::getName(){
    return this->name;
}
void miner::setName(string name){
    this->name= name;
}
transaction * miner::getTransaction(){
    return this->tentativeBlock.trans;
}
int miner:: getSize(){
    return size;
}
int miner::getCountOfAttemptTransactions(){
    return this->sucessAttemptTransactions;
}
int miner::getCountOfMinnedBlocks(){
    return this->minedBlock;
}
miner::miner( string name)
{
    
    this->name=name;
    this->isActive=true;
}
bool miner::isReady(){
    return this->waitTicks==0;
}
miner::miner(){
    isActive=false;
}
miner::~miner()
{
}
void miner::flush( transaction removeTrans[]){
    this->tentativeBlock.prev=nullptr;
    /*removing common elements*/
    for (int i = 0; i < MAX_TRANSACTION_COUNT; i++)
    {
        for (int j = 0; j < MAX_TRANSACTION_COUNT; j++)
        {
           if(this->tentativeBlock.trans[i].transactionId==removeTrans[j].transactionId){
                this->tentativeBlock.trans[i].transactionId=-1;
                size--;
           }
        }

    }
    /* moving elements to top*/
    int count=0;
    for(int i=0;i<MAX_TRANSACTION_COUNT;i++){
        if(this->tentativeBlock.trans[i].transactionId!=-1){
           this->tentativeBlock.trans[count]=this->tentativeBlock.trans[i];
           count++; 
        }
        else{
            this->tentativeBlock.trans[i].transactionId=-1;
        }
    }

    this->size=0;
}
bool miner::validateTransaction(transaction tran ){
    bool senderFlag=false;
    for(int i=0;i<MAX_CUSTOMER_COUNT;i++){
        if(cust[i].id==tran.sender.id && cust[i].dollars>tran.amount){
            //valid sender.
            senderFlag=true;
        }
    }
    return senderFlag;
}
void miner::addTransactionToTentativeBlock(transaction tran){
    bool isAddNewCandidateBlock=true;
    sucessAttemptTransactions++;
    writeRequestsToFile(tran,this->name);
    for( long unsigned int i =0;i<candidateBlocks.size();i++){
        bool uniquecustomerFlag=true;
        for(int j=0;j<candidateBlocks[i].transactionCount;j++){
            if(candidateBlocks[i].b.trans[j].sender.id==tran.sender.id){
                uniquecustomerFlag=false;
            }
        }
        if(uniquecustomerFlag){
            candidateBlocks[i].b.trans[candidateBlocks[i].transactionCount]=tran;
            candidateBlocks[i].transactionCount++;
            isAddNewCandidateBlock=false;
            if(candidateBlocks[i].transactionCount==MAX_TRANSACTION_COUNT){
                //try generating block
                 addBlock(candidateBlocks[i].b);
            }
            break;
        }
    }
    if(isAddNewCandidateBlock){
        block b;
        b.trans[0]=tran;
        candidateblock c;
        c.b=b;
        c.transactionCount++;
        candidateBlocks.push_back(c);
    }
}
void miner::addBlock(block b){
    // block * temp=latestBlock;
    block * realBlock = new block;
    blockCounter++;
    //copying tentative block to real block
    for (int i = 0; i < MAX_TRANSACTION_COUNT; i++)
    {
        realBlock->trans[i]=b.trans[i];
        //update customers 
        updateCustomer(b.trans[i]);
    }
    realBlock->prev=latestBlock;
    realBlock->minedBy=this->name;
    realBlock->blockId=blockCounter;
    realBlock->prevHash=hashBlock();
    latestBlock=realBlock;
    minedBlock++;
    writeBlocktoFile(*latestBlock);
}
string miner::hashstring(string toBeHashed){
    
    CryptoPP::SHA256 hash;
    CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];

    hash.CalculateDigest(digest, (const CryptoPP::byte*) toBeHashed.c_str(), toBeHashed.length());

    CryptoPP::HexEncoder encoder;
    string output;

    encoder.Attach(new CryptoPP::StringSink(output));
    encoder.Put(digest, sizeof(digest));
    encoder.MessageEnd();
    return output;
}
string miner::hashBlock(){
    
    string data= "";
    data=data+to_string(latestBlock->blockId);
    data=data+latestBlock->minedBy;
    data=data+latestBlock->prevHash;
    for (long unsigned int i = 0; i < MAX_TRANSACTION_COUNT; i++)
    {
        data=data+latestBlock->trans[i].sender.name+latestBlock->trans[i].reciever.name;
    }
    return hashstring(data);
}
void miner::updateCustomer(transaction t){
      
    int senderIndex=0;
    int recieverIndex=0;
     for (long unsigned int  i = 0; i < cust.size(); i++){
        if(t.sender.id==cust[i].id){
            senderIndex=i;
        }
        if(t.reciever.id==cust[i].id){
            recieverIndex=i;
        }
    }
    switch (t.type)
    {
    case T0:
        //deposit
        cust[senderIndex].dollars=cust[senderIndex].dollars+t.amount;
        break;
    case T1:
        //withdraw
        cust[senderIndex].dollars=cust[senderIndex].dollars-t.amount;
        break;
    case T2:
    //transfer amount
        cust[recieverIndex].dollars=cust[recieverIndex].dollars+t.amount;
        cust[senderIndex].dollars=cust[senderIndex].dollars-t.amount;
        break;
    case T3:
        //do nothing null transations.
        break;
    default:
         //do nothing invalid transaction type.
        break;
    }
}

/* Definations*/
void initCustomer(){
    customer A;
    A.id=cust.size();
    A.name="A";
    A.dollars=10000;
    A.isActive=true;
    cust.push_back(A);

    customer B;
    B.id=cust.size();
    B.name="B";
    B.dollars=10000;
    B.isActive=true;
    cust.push_back(B);

    customer C;
    C.id=cust.size();
    C.name="C";
    C.dollars=10000;
    C.isActive=true;
    cust.push_back(C);
    customer D;
    D.id=cust.size();
    D.name="D";
    D.dollars=10000;
    D.isActive=true;
    cust.push_back(D);
    for (int i = 0; i < MAX_TEMPORARY_CUSTOMER; i++)
    {
        customer temporaryCustomer;
        temporaryCustomer.id=cust.size();
        temporaryCustomer.dollars=10000;
        temporaryCustomer.isActive=false;
        temporaryCustomer.name="customer "+to_string(cust.size()-ALWAYS_ACTIVE_CUSTOMER_COUNT);
        cust.push_back(temporaryCustomer);
    }
    
}
void openAllFiles(){
    eventFile.open("events.csv");
    requestFile.open("requests.csv");
    blockFile.open("blockchain.csv");
}
void closeAllFiles(){
    eventFile.close();
    requestFile.close();
    blockFile.close();
}
void writeCustomerEventsToFile(event e){
    eventFile<<e.tick<<",customer: "<<e.cust.name <<",Change to ,";
    if(e.cust.isActive){
        eventFile<<"inactive"<<endl;
    }
    else{
        eventFile<<"active"<<endl;
    }
}
void writeMinerEventsToFile(event e){
    eventFile<<e.tick<<",Miner : "<<e.m->getName() <<",Change to ,";
    if(e.m->isActive){
        eventFile<<"inactive"<<endl;
    }
    else{
        eventFile<<"active"<<endl;
    }
}
void writeTransactionEventsToFile(transaction tran){
    eventFile<<tran.tick<<","<<tran.transactionId<<","<<printTransType(tran.type)<<","<<tran.sender.name<< 
    ","<<tran.reciever.name<<","<<tran.amount<<endl;
}
void writeRequestsToFile(transaction tran, string minerName){
    requestFile<<tran.tick<<","<<minerName<<","<<tran.transactionId<<","<<printTransType(tran.type)<<","<<tran.sender.name<< 
    ","<<tran.reciever.name<<","<<tran.amount
    <<endl;
}
void writeBlocktoFile(block b){
    for (int  i = 0; i < MAX_TRANSACTION_COUNT; i++)
    {
        blockFile<<b.trans[i].tick<<","<<b.minedBy<<","<<b.blockId<<
        ","<<b.trans[i].transactionId<<","<<printTransType(b.trans[i].type)
        <<","<<b.trans[i].sender.name<< 
        ","<<b.trans[i].reciever.name<<","<<b.trans[i].amount<<","<<","<<b.prevHash<<endl;
    }
}
string printTransType(transactionType type){
    switch (type)
    {
    case T0:
        return "T0";
        break;
    case T1:
        return "T1";
        break;
    case T2:
        return "T2";
        break;
    case T3:
        return "T3";
        break;
    default:
        return "Terror";
        break;
    }
    return "";
}
customer getActiveCustomer(){
    int randIndex=rand()%cust.size();
    long unsigned int softBreakCounter=0;
    while(!cust[randIndex].isActive){
        randIndex=rand()%cust.size();
        softBreakCounter++;
        if(softBreakCounter>cust.size()){
            //soft break infinite loop
            cust[randIndex].isActive=true;
            return cust[randIndex];
        }
    }
    return cust[randIndex];
    
}
miner * getActiveMiner(){
    int randIndex=rand()%miners.size();
    long unsigned int softBreakCounter=0;
    while(!cust[randIndex].isActive){
        randIndex=rand()%miners.size();
        softBreakCounter++;
        if(softBreakCounter>miners.size()){
            //soft break infinite loop
            miners[randIndex].isActive=true;
            return &miners[randIndex];
        }
    }
    return &miners[randIndex];
}
miner * getRandomMinerToFlipStatus(){
    int randomIndex=rand()%(miners.size()-ALWAYS_ACTIVE_MINER_COUNT)+ALWAYS_ACTIVE_MINER_COUNT;
    return &miners[randomIndex];
}
customer getRandomCustomerToFlipStatus(){

    int randomIndex=(rand()%(cust.size()-ALWAYS_ACTIVE_CUSTOMER_COUNT)) +ALWAYS_ACTIVE_CUSTOMER_COUNT;
    return cust[randomIndex];
}
event generateSingleEvent(){
    bool isTransaction=rand()%2;
    event e;
    if (isTransaction)
    {
        transaction tran;
        tran.transactionId=eventCount;
        tran.tick=tick;
        tran.sender=getActiveCustomer();
        tran.amount=rand()%100+50;
        double randomTransactionProbability= (double) rand()/(double) RAND_MAX;
        
        if(randomTransactionProbability<=0.1){	//10 percent t0 deposit
            tran.type=T0;
        }
        else if (randomTransactionProbability<=0.2){ //10 percent t1 withdraw
            tran.type=T1;
        }
        else if(randomTransactionProbability<=0.3){  //10 percent t2 trnsfer
            tran.type=T2;
            tran.reciever=getActiveCustomer();
        }
        else{						//70 percent t3 nulll transaction
            tran.type=T3;
        }
       
        e.tran=tran;
        e.isTransaction=true;
        e.isMiner=false;
        e.isCustomer=false;
        writeTransactionEventsToFile(tran);
    }
    else{
        /*add or remove miner*/
        e.tick=tick;
        e.isTransaction=false;
        bool isMinerOrCustomer=rand()%2;
        if(isMinerOrCustomer){
            e.isMiner=true;
            e.isCustomer=false;
            e.m=getRandomMinerToFlipStatus();
            writeMinerEventsToFile(e);
        }
        else{
            e.isCustomer=true;
            e.isMiner=false;
            e.cust=getRandomCustomerToFlipStatus();
            writeCustomerEventsToFile(e);
        }

    }
    eventCount++;
    tick=tick+(rand()%10);
    return e;
}

void generateEventQueue(int size){
    int counter=0;
    while ( counter<size)
    {
        eventQueue.push(generateSingleEvent());
        counter++;
    }
    
}

void triggerEvent(){
    event e=eventQueue.front();
    if(e.isTransaction){
        for(long unsigned int i=0;i<miners.size();i++){
        //add to tentative block
            if(miners[i].isActive){
                if(miners[i].validateTransaction(e.tran)){
                    miners[i].addTransactionToTentativeBlock(e.tran);
                }
            }
        }
    }
    else if(e.isCustomer){
        for(long unsigned int i =0;i<cust.size();i++){
            if(cust[i].id==e.cust.id){
                cust[i].isActive=!(cust[i].isActive);
            }
        }
    }
    else if(e.isMiner){
        for(long unsigned int i =0;i<miners.size();i++){
            if(miners[i].getName()==e.m->getName()){
                miners[i].flipStatus();
            }
        }
    }
    eventQueue.pop();
    int randomNumberOfEvent=rand()%4;
    generateEventQueue(randomNumberOfEvent);
}