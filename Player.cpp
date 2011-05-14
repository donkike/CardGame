#include<iostream>
#include<vector>
#include<string>
#include<signal.h>
#include<SimpleSemaphore.h>
#include<SharedMemory.h>
#include<Cards.h>

using namespace std;

enum { WAITING, PLAYING };
enum {WIN, LOSE, TIE};

SimpleSemaphore ready("/ready", 0);
SimpleSemaphore zync("/sync", 0);
SimpleSemaphore numPlayers("/numPlayers", 0);
SharedMemory<int> mem_status("/status");
SharedMemory<card> mem_current("/current");
SharedMemory<int> mem_last("/last");
SharedMemory<char> mem_tmp("/tmp");
SharedMemory<int> mem_score("/score");

void printUsage() {
	cout << "Usage:" << endl;
	cout << "  ./Player [--cheat]" << endl;
	cout << "Type -h or --help to show this message" << endl;
};

class Player
{
public:
	Player ();
	virtual ~Player ();
	void play(bool cheat);

private:
	string name;
	deck myDeck;
	bool cheat;
};

Player::Player() {
	
};

Player::~Player() {
	
};

void Player::play(bool cheat) {	
	
	name = "player_";
	myDeck = generateDeck();
	
	this->cheat = cheat;
	
	int &status = mem_status();
	card &current = mem_current();
	char &tmp = mem_tmp();
	int &last = mem_last();
	int &score = mem_score();
	
	numPlayers.Signal();
	cout << "Waiting for game..." << endl;;
	
	ready.Wait();
	name.append(1, tmp);	
	SimpleSemaphore turn("/" + name, 0);
	cout << "Semaphore is /" << name << endl;
	zync.Signal();	
	
	cout << "The game has begun" << endl;
	
	int turns = 0;
	
	while (status == PLAYING) {
		turn.Wait();
		
		if (status != PLAYING) break;
		
		cout << "Is my turn " << (++turns) << endl;
		
		int index = rand() % myDeck.size();
		current = myDeck[index];
		if (!cheat) myDeck.erase(myDeck.begin() + index);
		current.print();
		
		ready.Signal();
		turn.Wait();
		switch(last) {
			case(WIN):
				cout << "You win!" << endl;
				break;
			case(LOSE):
				cout << "You lose..." << endl;
				break;
			case(TIE):
				cout << "Tie" << endl;
				break;
		}
		zync.Signal();
		
		cout << endl;
	}

	turn.Wait();
	
	cout << "Your score is " << score << endl;
	switch(last) {
		case(WIN):
			cout << "You have won!" << endl;
			break;
		case(LOSE):
			cout << "You have lost" << endl;
			break;
		case(TIE):
			cout << "It's a tie" << endl;
			break;
	}
	
	zync.Signal();
	
	cout << endl;
};


int main (int argc, char const *argv[]) 
{	
	srand(time(NULL));
	
	Player player;
	if (argc > 1) {
		if (strcmp(argv[1], "--cheat") == 0) {
			cout << "Begin game with cheating" << endl;		
			player.play(true);
		} else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
			printUsage();
	} else {
		cout << "Begin game without cheating" << endl;
		player.play(false);		
	}
	return 0;
}