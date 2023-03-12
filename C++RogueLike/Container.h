#ifndef PROJECT_PATH_CONTAINER_H_
#define PROJECT_PATH_CONTAINER_H_

class Container
{
public:
	int invSize = 0;

	std::vector<std::shared_ptr<Actor>> inventoryList;

	Container(int invSize);
	~Container();

	// checks that the container is not full.
	bool add(Actor& actor);
	void remove(Actor& actor);

	void load(TCODZip& zip);
	void save(TCODZip& zip);

	void print_container(std::vector<std::shared_ptr<Actor>> container);
};

#endif // !PROJECT_PATH_CONTAINER_H_