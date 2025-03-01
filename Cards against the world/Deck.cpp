#include "Deck.h"
#include <fstream>
#include <string>

Deck::Deck()
{
}
std::string Deck::getCard(std::size_t pos, bool black__)
{
	try {
		return black__ ? black_.at(pos).text : white_.at(pos);
	}
	catch (...) {
		return std::string("error: validate your deck, cound not find ") + (!black__ ? "white " : "black ") + "card, id: " + std::to_string(pos);
	}
}
bool Deck::getDouble(std::size_t pos)
{
	try {
		return black_.at(pos).doubl;
	}
	catch (...) {
		return false;
	}
}
Deck::card Deck::checkIfNewCard(std::string& line) {
	if (line.size() < 3)
		return card::none;
	if (line.substr(0, 3) == "//b")
		return card::black;
	if (line.substr(0, 3) == "//w")
		return card::white;
	return card::none;
}
int Deck::getNumber(std::string & line)
{
	line.erase(0, 3);
	return std::stoi(line);
}
bool Deck::load(std::experimental::filesystem::path name)
{
	white_.clear();
	black_.clear();
	pathToDeck = name;
	std::ifstream file("Decks" / pathToDeck);
	std::string buff, wholeCard;
	whiteCards = 0;
	BlackCards = 0;
	try {

		int whiteExpected, blackExpected;
		if (!std::getline(file, buff))//get number of black cards
			return false;
		if(!std::getline(file, buff))//get number of black cards
			return false;
		if (buff.size() < 1)
			return false;
		blackExpected = std::stoi(buff);
		if (!std::getline(file, buff))//get number of white cards
			return false;
		if (buff.size() < 1)
			return false;
		whiteExpected = std::stoi(buff);
		black_.resize(blackExpected);
		white_.resize(whiteExpected);
		while (std::getline(file, buff)) {
			card check = checkIfNewCard(buff);
			if (check == card::none) {
				wholeCard += buff + " ";
				continue;
			}
			int index = getNumber(buff);
			if (check == card::black) {
				if (!black_.at(index).text.empty()) {
					return false;
				}
				int found = 0;
				for (auto& x : wholeCard) {
					if (x == '_')
						found++;
				}
				black_.at(index).text = wholeCard;
				black_.at(index).doubl = found > 2;
				BlackCards++;
				wholeCard.clear();
			}
			else if (check == card::white) {
				if (!white_.at(index).empty()) {
					return false;
				}
				white_.at(index) = wholeCard;
				whiteCards++;
				wholeCard.clear();
			}

		}
		for (auto& x : white_)
			if (x.empty())
				return false;
		for (auto& x : black_)
			if (x.text.empty())
				return false;
		if (white_.empty() || black_.empty())
			return false;
		return true;
	}
	catch (...) {
		return false;
	}
}


Deck::~Deck()
{
}
