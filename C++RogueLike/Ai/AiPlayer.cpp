// file: AiPlayer.cpp
#include <curses.h>
#include <libtcod.h>
#include <unordered_map>
#include <format>
#include <gsl/util>

#include "Ai.h"
#include "AiPlayer.h"
#include "../Game.h"
#include "../Menu/Menu.h"
#include "../Controls/Controls.h"
#include "../Actor/Actor.h"
#include "../Items.h"

//==INVENTORY==
constexpr int INVENTORY_HEIGHT = 29;
constexpr int INVENTORY_WIDTH = 30;

//==PLAYER_AI==
struct PossibleMoves
{
	std::unordered_map<Controls, Vector2D> moves = {
		{Controls::UP, {-1, 0}},
		{Controls::DOWN, {1, 0}},
		{Controls::LEFT, {0, -1}},
		{Controls::RIGHT, {0, 1}},
		{Controls::UP_LEFT, {-1, -1}},
		{Controls::UP_RIGHT, {-1, 1}},
		{Controls::DOWN_LEFT, {1, -1}},
		{Controls::DOWN_RIGHT, {1, 1}}
	};
} m;

void AiPlayer::update(Creature& owner)
{
	const Controls key = static_cast<Controls>(game.keyPress);
	Vector2D moveVector{ 0, 0 };

	// Check for movement first
	if (m.moves.find(key) != m.moves.end())
	{
		moveVector = m.moves.at(key);
		game.gameStatus = Game::GameStatus::NEW_TURN;
	}
	else
	{
		// if the key is not a movement key, call the action
		call_action(owner, key);
	}

	if (isWaiting) // when waiting tiles are triggered
	{
		isWaiting = false;
		game.map->tile_action(owner, game.map->get_tile_type(owner.position));
		look_on_floor(owner.position);
	}

	// if moving
	if (moveVector.x != 0 || moveVector.y != 0)
	{
		Vector2D targetPosition = owner.position + moveVector;
		game.map->tile_action(owner, game.map->get_tile_type(targetPosition));
		look_to_move(owner, targetPosition); // must check colissions before creature dies from attack
		look_to_attack(targetPosition, owner);
		look_on_floor(targetPosition);
		if (shouldComputeFOV)
		{
			shouldComputeFOV = false; // reset flag
			game.map->compute_fov();
		}
	}
}

void AiPlayer::load(const json& j)
{
	/*type is assigned in Ai::create()*/
}

void AiPlayer::save(json& j)
{
	j["type"] = static_cast<int>(AiType::PLAYER);
}

void AiPlayer::move(Creature& owner, Vector2D target)
{
	owner.position = target;
}

void AiPlayer::pick_item(Creature& owner)
{
	owner.pick();
}

void AiPlayer::drop_item(Creature& owner)
{
	owner.drop();
}

bool AiPlayer::is_pickable_at_position(const Actor& actor, const Actor& owner) const
{
	return actor.position == owner.position;
}

void AiPlayer::display_inventory_items(WINDOW* inv, const Creature& owner) noexcept
{
	int shortcut = 'a';
	int y = 1;
	try
	{
		for (const auto& item : owner.container->inv)
		{
			if (item != nullptr)
			{
				mvwprintw(inv, y, 1, "(%c) %s", shortcut, item->actorData.name.c_str());
				// if the actor is equipped, print a star
				if (item->has_state(ActorState::IS_EQUIPPED))
				{
					int nameLength = strlen(item->actorData.name.c_str()) + 5;
					mvwprintw(inv, y, nameLength, "*");
				}
			}
			y++;
			shortcut++;
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "Error: PlayerAi::displayInventoryItems(WINDOW* inv, const Actor& owner). " << e.what() << std::endl;
		exit(-1);
	}
}

void AiPlayer::display_inventory(Creature& owner)
{
	refresh();

	WINDOW* inv = newwin(INVENTORY_HEIGHT, INVENTORY_WIDTH, 0, 0);

	box(inv, 0, 0);
	mvwprintw(inv, 0, 0, "Inventory");

	try
	{
		if (owner.container->inv.size() > 0)
		{
			display_inventory_items(inv, owner);
		}
		else
		{
			const int y = 1;
			mvwprintw(inv, y, 1, "Your inventory is empty.");
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "Error: PlayerAi::display_inventory(Actor& owner). " << e.what() << std::endl;
		exit(-1);
	}

	wrefresh(inv);

	const int inventoryInput = getch();
	if (inventoryInput == static_cast<int>(Controls::ESCAPE))
	{
		delwin(inv);
		clear();
	}
	else if (inventoryInput >= 'a' && inventoryInput <= 'z')
	{
		const auto& actor = chose_from_inventory(owner, inventoryInput);
		if (actor)
		{
			actor->pickable->use(*actor, owner);
			/*game.gameStatus = Game::GameStatus::NEW_TURN;*/
		}
		delwin(inv);
		clear();
	}
	else
	{
		delwin(inv);
		clear();
	}
}

Item* AiPlayer::chose_from_inventory(Creature& owner, int ascii)
{
	game.log("You chose from inventory");
	if (owner.container != nullptr)
	{
		const size_t index = ascii - 'a';
		if (index >= 0 && index < owner.container->inv.size())
		{
			return owner.container->inv.at(index).get();
		}
		else
		{
			// if the index is out of bounds
			return nullptr;
		}
	}
	else
	{
		game.log("Error: choseFromInventory() called on actor with no container.");
		exit(EXIT_FAILURE);
	}
}

void AiPlayer::look_on_floor(Vector2D target)
{
	// look for corpses or items
	for (const auto& i : game.container->inv)
	{
		if (i)
		{
			if (i->position == target)
			{
				game.appendMessagePart(WHITE_PAIR, std::format("There's a {} here\n", i->actorData.name));
				game.finalizeMessage();
			}
		}
	}
}

bool AiPlayer::look_to_attack(Vector2D& target, Creature& owner)
{
	// look for living actors to attack
	for (const auto& c : game.creatures)
	{
		if (c)
		{
			if (!c->destructible->is_dead() && c->position == target)
			{
				owner.attacker->attack(owner, *c);
				return false;
			}
		}
	}
	return true;
}


void AiPlayer::look_to_move(Creature& owner, const Vector2D& targetPosition)
{
	if (!game.map->is_collision(owner, game.map->get_tile_type(targetPosition), targetPosition))
	{
		move(owner, targetPosition);
		shouldComputeFOV = true;
	}
}


void AiPlayer::call_action(Creature& owner, Controls key)
{
	switch (key)
	{
	case Controls::WAIT:
	case Controls::WAIT_ARROW_NUMPAD:
	{
		game.gameStatus = Game::GameStatus::NEW_TURN;
		isWaiting = true;
		break;
	}

	case Controls::HIT_SELF:
	{
		game.player->attacker->attack(*game.player, *game.player);
		game.gameStatus = Game::GameStatus::NEW_TURN;
		break;
	}

	case Controls::MOUSE:
	{
		std::cout << "mouse" << std::endl;
		request_mouse_pos();
		break;
	}

	case Controls::PICK:
	case Controls::PICK_SHIFT_STAR:
	case Controls::PICK_NUMPAD:
	{
		pick_item(owner);
		game.gameStatus = Game::GameStatus::NEW_TURN;
		break;
	}

	case Controls::DROP:
	{
		drop_item(owner);
		game.gameStatus = Game::GameStatus::NEW_TURN;
		break;
	}

	case Controls::INVENTORY:
	{
		display_inventory(owner);
		break;
	}

	case Controls::QUIT:
	{
		game.run = false;
		game.message(WHITE_PAIR, "You quit the game ! Press any key ...", true);
		break;
	}

	case Controls::ESCAPE: // if escape key is pressed bring the game menu
	{
		game.menus.push_back(std::make_unique<Menu>());
		break;
	}

	case Controls::DESCEND:
	{
		if (game.stairs->position == owner.position)
		{
			game.next_level(); // sets state to STARTUP
		}
		break;
	}

	case Controls::TARGET:
	{
		game.target();
		break;
	}

	case Controls::CHAR_SHEET:
	{
		game.display_character_sheet();
		break;
	}

	case Controls::DEBUG:
	{
		game.display_debug_messages();
		break;
	}

	case Controls::REVEAL:
	{
		game.map->reveal();
		break;
	}

	case Controls::REGEN:
	{
		game.map->regenerate();
		break;
	}

	default:
		break;
}
}
