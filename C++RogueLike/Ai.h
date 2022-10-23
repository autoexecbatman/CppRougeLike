#ifndef PROJECT_PATH_AI_H_
#define PROJECT_PATH_AI_H_

//==AI==
class Ai : public Persistent
{
public:
	virtual ~Ai() {};
	virtual void update(Actor* owner) = 0;
	static Ai* create(TCODZip& zip);
protected:
	enum class AiType 
	{
		MONSTER, CONFUSED_MONSTER, PLAYER
	};
};

//==MONSTER_AI==
class MonsterAi : public Ai
{
public:
	MonsterAi();
	void update(Actor* owner);

	void load(TCODZip& zip);
	void save(TCODZip& zip);
	
protected:
	int moveCount = 0;

	void moveOrAttack(Actor* owner, int targetx, int targety);
};

//==PLAYER_AI==
class PlayerAi : public Ai
{
public:
	int xpLevel;
	PlayerAi();
	int getNextLevelXp();
	void update(Actor* owner);
	void load(TCODZip& zip);
	void save(TCODZip& zip);
	
	void pick_item(Actor* owner);
	void display_inventory(Actor* owner);
	
protected:
	/*void handleActionKey(Actor* owner, int ascii);*/
	Actor* choseFromInventory(Actor* owner, int ascii);
	bool moveOrAttack(Actor* owner, int targetx, int targety);
	
};

//====
class ConfusedMonsterAi : public Ai
{
public:
	ConfusedMonsterAi(int nbTurns, Ai* oldAi);
	void update(Actor* owner);
	void load(TCODZip& zip);
	void save(TCODZip& zip);
	
protected:
	int nbTurns;
	Ai* oldAi;
};

#endif // !PROJECT_PATH_AI_H_