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

volatile bool keepPlaying = true;

void SignalHandler(int sigNum) {
  if (sigNum == SIGQUIT || sigNum == SIGINT) {
    keepPlaying = false;
  }
};

void printUsage() {
	cout << "Usage:" << endl;
	cout << "  ./Controller [--slow]" << endl;
	cout << "Type -h or --help to show this message" << endl;
};

class Controller
{
public:
	Controller ();
	virtual ~Controller ();
	void begin(bool slow);
	bool cheated(int player, card &c);

private:
	int points[2];
	int turns;
	int winner;
	deck played[2];
	bool cheat;
};

Controller::Controller() {
	
};

Controller::~Controller() {
	
};

bool Controller::cheated(int player, card &c) {
	for (int i = 0; i < played[player].size(); i++) {
		if (played[player][i] == c) return true;
	}
	return false;
};

void Controller::begin(bool slow) {
	
	cout << "Waiting for enough players..." << endl;
	numPlayers.Wait();
	cout << "Waiting for another player..." << endl;
	numPlayers.Wait();
	
	played[0].clear();
	played[1].clear();
	
	int &status = mem_status();
	card &current = mem_current();
	char &tmp = mem_tmp();
	int &last = mem_last();
	int &score = mem_score();
	
	status = WAITING;
	
	memset(points, 0, sizeof(points));
	
	status = PLAYING;
	
	cout << "Beginning game..." << endl;

	tmp = 'a';
	SimpleSemaphore firstPlayer("/player_a", 0);
	ready.Signal();
	zync.Wait();
	
	if (slow) sleep(1);	

	tmp = 'b';
	SimpleSemaphore secondPlayer("/player_b", 0);
	ready.Signal();
	zync.Wait();
	
	int acum = 0;
	turns = 0;
	winner = -1;
	cheat = false;
	
	cout << "The game has begun" << endl << endl;
	
	while (keepPlaying && turns < 52 && status == PLAYING) {
		cout << "Turn is " << (turns + 1) << endl;
		
		firstPlayer.Signal();
		ready.Wait();
		card card1 = current;
		
		cout << "Player1:" << endl;
		card1.print();
		
		if (!cheated(0, card1))
			played[0].push_back(card1);
		else {
			cout << "Player1 has cheated" << endl;
			winner = 1;
			cheat = true;
			break;
		}		
		
		secondPlayer.Signal();
		ready.Wait();
		card card2 = current;
		
		cout << "Player2:" << endl;
		card2.print();
		
		if (!cheated(1, card2))
			played[1].push_back(card2);
		else {
			cout << "Player2 has cheated" << endl;
			winner = 0;
			cheat = true;
			break;
		}
		
		if (card1 < card2) {
			points[0] += acum + 1;
			cout << "Player1 wins and gets " << (acum + 1) << " point(s)" << endl;
			last = WIN;
			firstPlayer.Signal();
			zync.Wait();
			last = LOSE;
			secondPlayer.Signal();
			zync.Wait();
			acum = 0;
		} else if (card1 > card2) {
			points[1] += acum + 1;
			cout << "Player2 wins and gets " << (acum + 1) << " point(s)" << endl;
			last = LOSE;
			firstPlayer.Signal();
			zync.Wait();
			last = WIN;
			secondPlayer.Signal();
			zync.Wait();
			acum = 0;
		} else {
			acum++;
			cout << "Tie" << endl;
			last = TIE;
			firstPlayer.Signal();
			zync.Wait();
			last = TIE;
			secondPlayer.Signal();
			zync.Wait();
		}
		turns++;
		cout << endl;
		if(slow) sleep(1);
	}
	status = WAITING;
	firstPlayer.Signal();
	secondPlayer.Signal();
	
	cout << endl << "Game over" << endl;
	
	if (!cheat) {
		if (points[0] > points[1]) winner = 0;
		else if (points[0] < points[1]) winner = 1;
		cout << endl << " === Summary === " << endl;
		cout << "Score Player1: " << points[0] << endl;
		cout << "Score Player2: " << points[1] << endl;
	}
	
	cout << endl;
	
	if (winner != -1) {
		cout << "Player" << (winner + 1) << " has won" << endl;
		last = winner;
		score = points[0];
		firstPlayer.Signal();
		zync.Wait();
		last = (winner + 1) % 2;
		score = points[1];
		secondPlayer.Signal();
		zync.Wait();
	} else { 
		cout << "It's a tie" << endl;	
		last = TIE;
		score = points[0];
		firstPlayer.Signal();
		zync.Wait();
		last = TIE;
		score = points[1];
		secondPlayer.Signal();
		zync.Wait();
	}
	
	cout << endl;
};


int main (int argc, char const *argv[])
{
	srand(time(NULL));
	struct sigaction sa;
  sa.sa_handler = SignalHandler;
  sa.sa_flags = 0;

  sigaction(SIGQUIT, &sa,NULL);
  sigaction(SIGINT, &sa,NULL);

	bool slow = false;

	if (argc > 1) {
		if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
			printUsage();
			return 0;
		} else if(strcmp(argv[1], "--slow") == 0) slow = true;
	}
	
	cout << "Starting controller";
	if (slow) cout << " with slow option";
	cout << endl;

	Controller controller;
	
	while (keepPlaying) {
		controller.begin(slow);
		if (keepPlaying) {
			cout << "Waiting 5 seconds before next game" << endl;
			sleep(5);
		}
	}
	
	return 0;
}