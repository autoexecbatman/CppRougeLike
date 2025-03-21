// file: AiShopkeeper.h
#pragma once

#include <span>

#include "Ai.h"

class Item;
class Player;
class Actor;

class AiShopkeeper : public Ai
{
private:
	int moveCount = 0;
	void update(Creature& owner) override;
	void load(const json& j) override;
	void save(json& j) override;
	int calculate_step(int positionDifference);
	void moveToTarget(Actor& owner, int targetx, int targety);
public:
	AiShopkeeper();
protected:
	void moveOrTrade(Creature& owner, int targetx, int targety);
public:
	void trade(Creature& shopkeeper, Creature& player);
};
// file: AiShopkeeper.h
