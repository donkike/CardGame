#include<vector>

using namespace std;

struct card {
	//const string[4] names = {"spades", "hearts", "diamonds", "clubs"};
	int value, suit;
	card(int v, int s) { value = v, suit = s; }
	void print() { cout << "card :: value : " << value << " - suit : " << suit << endl; }
	bool operator<(card &c) { return value < c.value; }
	bool operator>(card &c) { return value > c.value; }
	bool operator==(card &c) { return value == c.value && suit == c.suit; }
}; 	

typedef vector<card> deck;

deck generateDeck() {
	deck theDeck;
	for (int i = 1; i <= 4; i++) {
		for (int j = 1; j <= 13; j++)
			theDeck.push_back(card(j, i));
	}
	return theDeck;
};



