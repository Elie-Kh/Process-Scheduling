#include <iostream>
#include <mutex>
#include <cmath>
#include <string>
#include <fstream>
#include <vector>
#include<thread>




using namespace std;
static int positionVector = 0;
mutex mtx;
static int oldRunning = 0;
static int numberOfProcesses = 0;
static int numberOfTotalProcesses = 0;

int max(int a, int b) {
    if (a >= b) {
        return a;
    }
    else {
        return b;
    }
}

int min(int a, int b) {
    if (a <= b) {
        return a;
    }
    else {
        return b;
    }
}



class Process {
private:
    int PID = 0;
    int arrivalTime = 0;
    int cpuBurst = 0;
    int priority = 0;
    int Tq = 0;
    Process *next = NULL;
    int lastFinish = 0;
    int bonus = 0;
    int sumOfWait = 0;
    string status = "";


    void print(int currenttime, string action) {

        cout << "Time " << currenttime << ", P" << PID << ", " << action;
    }

public:
    Process() {};
    ~Process() {
        next = NULL;
        status = "";
    }
    Process(int pid, int arrivaltime, int cpuburst, int prior) {
        PID = pid;
        arrivalTime = arrivaltime;
        cpuBurst = cpuburst;
        priority = prior;
        Tq = calcTq(prior);
    }

    //set and get functions for the private variables
    Process* getNext() const {
        return next;
    }
    void setNext(Process q) {
        next = &q;
    }
    int getPID() const {
        return PID;
    }
    int getBurst() const {
        return cpuBurst;
    }
    int getTq() const {
        return Tq;
    }
    string getStatus() const {
        return status;
    }
    int getArrival() const {
        return arrivalTime;
    }
    int getPrio() const {
        return priority;
    }

    //function to calculate Tq according to priority
    int calcTq(int prior) {
        if (prior < 100)
            return (140 - prior) * 20;
        else return (140 - prior) * 5;
    }

    int sumOfWaits(int currentTime) {
        sumOfWait = sumOfWait + (currentTime - lastFinish);
        return sumOfWait;
    }

    int calcBonus(int currentTime) {
        bonus = ceil(10 * sumOfWait / (currentTime - arrivalTime));
        return bonus;
    }

    void setNewPrio(int currenttime) {
        priority = max(100, min((priority - calcBonus(currenttime) + 5), 139));
        Tq = calcTq(priority);
        print(currenttime, "priority updated to ");
        cout << priority << endl;
    }





    void pause(int currentTime) {    //happens after one run
        lastFinish = currentTime;
        print(currentTime, "Paused");
        cout << endl;
    }
    void start(int currentTime) {    //happens when a process starts
        status = "Started";
        cpuBurst = cpuBurst - Tq;
        print(currentTime, "Started, Granted ");
        cout << Tq << endl;
        //oldRunning = PID;
    }

    void arrive(int currentTime) {   //happens when a process arrive
        status = "Arrived";
        print(currentTime, status);
        cout << endl;
    }

    void resume(int currentTime) {
        sumOfWait = sumOfWaits(currentTime);
        cpuBurst = cpuBurst - Tq;
        print(currentTime, "Resumed, Granted ");
        cout << Tq << endl;
        //oldRunning = PID;
    }

    void terminate(int currentTime) {
        numberOfProcesses--;
        print(currentTime, "Terminated");
        cout << endl;
    }

};

/* Check if position in the vector is bigger or equal to the number of processes. If yes, return because everything was added.
If not. When time is equal to arrival of a process, add the process.
Process adding checks if the slot is empty. If not empty, it will start acting as a linkedlist for the same priority.
if a prio 100 process exists and another process comes in, the 1st process points to the 2nd. */

void addToQueue(Process* (&queue)[140], vector<Process> &vect, int currentTime) {
    if (positionVector > numberOfProcesses) {
        return;
    }
    if (vect[positionVector].getArrival() == currentTime) {
        if (queue[vect[positionVector].getPrio()] == NULL) {
            queue[vect[positionVector].getPrio()] = &vect[positionVector];

            vect[positionVector].arrive(currentTime);
            positionVector++;
        }
        else {
            Process *temp1;
            Process temp2(vect[positionVector].getPID(), vect[positionVector].getArrival(), vect[positionVector].getBurst(), vect[positionVector].getPrio());
            temp1 = &temp2;
            while (temp1->getNext() != NULL) {
                temp1 = temp1->getNext();
            }
            temp1->setNext(vect[positionVector]);

            temp1 = temp1->getNext();
            Process tempo;
            temp1->setNext(tempo);
            queue[vect[positionVector].getPrio()] = temp1;
            vect[positionVector].arrive(currentTime);
            positionVector++;
        }
    }
}

void scheduler(Process* (&q0)[140], Process* (&q1)[140], bool flag, int currentTime, vector<Process> &vect) {
    if (numberOfProcesses == 0) {
        return;
    }

    else if (!flag) {
        if (positionVector < numberOfTotalProcesses) {
            if (currentTime == vect[positionVector].getArrival()) {
                addToQueue(q0, vect, currentTime);
            }
        }

        for (int i = 0; i < 140; i++) {
            if (q0[i] != NULL) {
                if (!q0[i]->getStatus().compare("Arrived")) {
                    mtx.lock();
                    q0[i]->start(currentTime);
                    int tempo = currentTime + q0[i]->getTq();

                    while (currentTime < tempo) {
                        currentTime += 100;
                        addToQueue(q1, vect, currentTime);

                    }
                    if (q0[i]->getBurst() <= 0) {
                        q0[i]->terminate(currentTime);
                        oldRunning = q0[i]->getPID();
                        q0[i]->~Process();
                        q0[i] = NULL;
                    }
                    else {
                        q0[i]->pause(currentTime);
                        q1[i] = q0[i];
                        oldRunning = q1[i]->getPID();
                    }

                    mtx.unlock();
                }
                else if (!q0[i]->getStatus().compare("Started")) {
                    mtx.lock();
                    q0[i]->resume(currentTime);
                    int tempo = currentTime + q0[i]->getTq();

                    while (currentTime < tempo) {
                        currentTime += 100;
                        addToQueue(q1, vect, currentTime);

                    }
                    if (q0[i]->getBurst() <= 0) {
                        q0[i]->terminate(currentTime);
                        oldRunning = q0[i]->getPID();
                        q0[i]->~Process();
                        q0[i] = NULL;
                    }
                    else {

                        q0[i]->pause(currentTime);
                        if (oldRunning == q0[i]->getPID()) {
                            q0[i]->setNewPrio(currentTime);
                        }

                        q1[i] = q0[i];
                        oldRunning = q1[i]->getPID();
                    }

                    mtx.unlock();
                }
            }//end if empty

        }//end for loop (you went through the whole array)
        flag = true;
        scheduler(q0, q1, flag, currentTime, vect);
    }//end if flag is 0
    else {
        for (int i = 0; i < 140; i++) {
            if (q1[i] != NULL) {
                if (!q1[i]->getStatus().compare("Arrived")) {
                    mtx.lock();
                    q1[i]->start(currentTime);
                    int tempo = currentTime + q1[i]->getTq();

                    while (currentTime < tempo) {
                        currentTime += 100;
                        addToQueue(q0, vect, currentTime);

                    }
                    if (q1[i]->getBurst() <= 0) {
                        q1[i]->terminate(currentTime);
                        oldRunning = q1[i]->getPID();
                        q1[i]->~Process();
                        q1[i] = NULL;
                    }
                    else {
                        q1[i]->pause(currentTime);
                        q0[i] = q1[i];
                        oldRunning = q0[i]->getPID();
                    }

                    mtx.unlock();
                }
                else if (!q1[i]->getStatus().compare("Started")) {
                    mtx.lock();
                    q1[i]->resume(currentTime);
                    int tempo = currentTime + q1[i]->getTq();

                    while (currentTime < tempo) {
                        currentTime += 100;
                        addToQueue(q0, vect, currentTime);

                    }
                    if (q1[i]->getBurst() <= 0) {
                        q1[i]->terminate(currentTime);
                        oldRunning = q1[i]->getPID();
                        q1[i]->~Process();
                        q1[i] = NULL;
                    }
                    else {
                        q1[i]->pause(currentTime);

                        if (oldRunning == q1[i]->getPID()) {
                            q1[i]->setNewPrio(currentTime);
                        }
                        q0[i] = q1[i];
                        oldRunning = q0[i]->getPID();
                    }

                    mtx.unlock();
                }
            }//end if empty
        }//end for loop (you went through the whole array)

        flag = false;
        scheduler(q0, q1, flag, currentTime, vect);
    }//end if flag is 1

}//end function scheduler

void sortVector(vector <Process> &a) {
    for (int i = 0; i < a.size() - 1; i++) {
        while (a[i + 1].getArrival() < a[i].getArrival()) {
            Process temp = a[i];
            a[i] = a[i + 1];
            a[i + 1] = temp;
        }
    }
}

//struct that takes the arguments to pass to the thread
struct args {
    Process* q0[140];
    Process* q1[140];
    bool flag;
    int currentTime;
    vector<Process> vect;
};
//pointer to the function to use for thread
void* schedule(args arr){
    scheduler(arr.q0, arr.q1, arr.flag, arr.currentTime, arr.vect);
    return NULL;
}


int main() {


    ifstream infile("inputs.txt");
    Process* q0[140];
    Process* q1[140];
    vector <Process> tempVect;
    args arr;

    for (int i = 0; i < 140; i++) {
        q0[i] = NULL;
        q1[i] = NULL;
    }
    for (int i = 0; i < 140; i++) {
        arr.q0[i] = q0[i];
        arr.q1[i] = q1[i];
    }
    bool flag = 0;
    string reading;
    int prio, burst, arrival, pid;
    getline(infile, reading, '\n');
    numberOfProcesses = stoi(reading, nullptr, numberOfProcesses);
    numberOfTotalProcesses = numberOfProcesses;
    while (!infile.eof()) {
        getline(infile, reading, ' ');
        infile >> pid >> arrival >> burst >> prio;
        Process temp(pid, arrival, burst, prio);

        tempVect.push_back(temp);

    }
    sortVector(tempVect);
    arr.vect = tempVect;
    arr.flag = flag;
    int timez = tempVect[positionVector].getArrival();
    arr.currentTime = timez;


    thread tester(&schedule, arr);
    tester.join();




    system("pause");
    return 0;
}
