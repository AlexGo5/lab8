#include <pthread.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <mutex>
#include <time.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

using namespace std;

string PRODUCER = "Producer";
string CONSUMER = "Consumer";

mutex mtx;

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
} arg;

int semid;

vector<string> messagesVector;

string generateRandomString() {
  const int length = rand() % 100 + 1;

  static const char alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

  string tempString;

  tempString.reserve(length);

  for (int i = 0; i < length; ++i) {
    tempString += alphanum[rand() % (sizeof(alphanum) - 1)];
  }
  
  return tempString;
}


void* producerHandler(void* tid) {   
  while(true) {
    sleep(2);

    struct sembuf sops[1];

    sops[0].sem_num = 0; 
		sops[0].sem_op = -1;
		sops[0].sem_flg = 0;
    semop(semid, sops, 1);

    string randomMessage = generateRandomString();

    messagesVector.push_back(randomMessage);

    cout << "\nProducer " + to_string(*((int *)tid));
    cout << "\nAdded message: " + randomMessage + "\n\n";

    sops[0].sem_num = 0;
		sops[0].sem_op = 1;
		sops[0].sem_flg = 0;
    semop(semid, sops, 1);

    sleep(3);
  }
}


void* consumerHandler(void* tid) {   
  while(1) {
    sleep(2);

    struct sembuf sops[2];
		
		sops[0].sem_num = 0; 
		sops[0].sem_op = -1;
		sops[0].sem_flg = 0;
    semop(semid, sops, 1);

    cout << "Consumer " + to_string(*((int *)tid)) + "\n";

    if (!messagesVector.empty()) {
      cout << "Recieved message: " + messagesVector.back() + "\n\n";

      messagesVector.pop_back();
    } else {
      cout << "No messages! Vector is empty!\n\n";
    }

    sops[0].sem_num = 0;
		sops[0].sem_op = 1;
		sops[0].sem_flg = 0;
    semop(semid, sops, 1);

    sleep(3);
  }
}

void createThread(
  vector<pthread_t*>& threads,
  vector<int*>& threadsIndexes,
  void* (*threadHandler)(void*),
  int& nextThreadIndex,
  string threadType
) {
  pthread_t* thread = new pthread_t;
  int* index = new int;
  *index = nextThreadIndex;

  threadsIndexes.push_back(index);
  threads.push_back(thread);
  if (pthread_create(&(*thread), NULL, threadHandler, index)) {
    cout << "\n\n" + threadType + " creating ERROR!!!\n\n" << endl;

    exit(EXIT_FAILURE);
  }

  cout << "\n\t" + threadType << " " + nextThreadIndex++ << " created!\n\n"; 
}

void cancelThread(
  vector<pthread_t*>& threads,
  vector<int*>& threadsIndexes,
  string threadType
) {
  if(threads.empty()) {
    cout << "\n\nNo active " + threadType + "\n\n";

    return;
  }

  pthread_t* thread = threads.back();
  int* threadNumber = threadsIndexes.back();

  cout << "\t" + threadType << *threadNumber <<" was cancelled!\n";

  pthread_cancel(*thread);

  threadsIndexes.pop_back();
  threads.pop_back();

  pthread_join(*thread, NULL);
  
  delete thread;
  delete threadNumber;
}

void cancelAllThread(
  vector<pthread_t*>& threads,
  vector<int*>& threadsIndexes,
  string threadType
) {
  while(threads.size()) {
    cancelThread(threads, threadsIndexes, threadType);
  }
}

int main(int argc, char** argv) {
  srand(time(NULL));

  vector<pthread_t*> producers;
  vector<pthread_t*> consumers;

  vector<int*> producersIndexes;
  vector<int*> consumersIndexes;

  int nextProducerIndex = 0;
  int nextConsumerIndex = 0;

  key_t key = 10;
	int flags = IPC_CREAT | 0666;
	
	semid = semget(key, 1, flags);

  arg.val = 1;
	semctl(semid, 0, SETVAL, arg);

  while(true) {
    cout << "\nPress key...\t";

    char c = getchar();
		getchar();

    switch(c) {   
      case 'p': 
      createThread(producers, producersIndexes, producerHandler, nextProducerIndex, PRODUCER);      
      break;

      case 'r': 
      cancelThread(producers, producersIndexes, PRODUCER);
      break;

      case 'c': 
      createThread(consumers, consumersIndexes, consumerHandler, nextConsumerIndex, CONSUMER);
      break;
      
      case 'd':
      cancelThread(consumers, consumersIndexes, CONSUMER);
      break;

      case 'q':
      cancelAllThread(producers, producersIndexes, PRODUCER);
      cancelAllThread(consumers, consumersIndexes, CONSUMER);
      exit(EXIT_SUCCESS);

      default:
      continue;

    }
  }

  return EXIT_SUCCESS;
}