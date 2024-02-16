#include <fstream>
#include <iostream>

#include "Scene_Wonder_Boy.h"
#include "Components.h"
#include "Physics.h"
#include "Utilities.h"
#include "MusicPlayer.h"
#include "Assets.h"
#include "SoundPlayer.h"
#include <random>
#include "Scene_Menu.h"

namespace
{
	std::random_device rd;
	std::mt19937 rng(rd());
}


Scene_Wonder_Boy::Scene_Wonder_Boy(GameEngine* gameEngine, const std::string& levelPath)
	: Scene(gameEngine)
{
	init(levelPath);
}


void Scene_Wonder_Boy::init(const std::string& levelPath)
{
	m_game->window().setView(m_game->window().getDefaultView());

	loadLevel(levelPath);
	registerActions();
	
	spawnPlayer();

	MusicPlayer::getInstance().play("level01");
	MusicPlayer::getInstance().setVolume(50);
}



void Scene_Wonder_Boy::sMovement(sf::Time dt)
{
	//player movement
	auto &tfm = m_player->getComponent<CTransform>();
	tfm.vel.x = 0.f;
	if (m_player->getComponent<CInput>().left)
	{
		tfm.vel.x -= 3; 
	}
	if (m_player->getComponent<CInput>().right)
	{
		tfm.vel.x += 3;
	}

	// gravity
	//tfm.vel.y += m_playerConfig.GRAVITY * dt.asSeconds();
	//tfm.vel.x *= m_playerConfig.SPEED;

	// facing direction
	if (tfm.vel.x < -0.1)
		m_player->getComponent<CState>().set(CState::isFacingLeft);
	if (tfm.vel.x > 0.1)
		m_player->getComponent<CState>().unSet(CState::isFacingLeft);


	// move all entities
	for (auto e : m_entityManager.getEntities()) {
		auto& tfm = e->getComponent<CTransform>();
		tfm.prevPos = tfm.pos;
		tfm.pos += tfm.vel;
	}
}

void Scene_Wonder_Boy::sAnimation()
{
	for (auto e : m_entityManager.getEntities())
	{
		auto &anim = e->getComponent<CAnimation>();
		if (anim.has) {
			anim.animation.update(anim.repeat);
			if (anim.animation.hasEnded()) { e->destroy(); }
		}
	}


}

void Scene_Wonder_Boy::onEnd()
{
	m_game->changeScene("MENU", std::make_shared<Scene_Menu>(m_game), true);
}


void Scene_Wonder_Boy::sRender()
{

	// background changes if paused
	static const sf::Color backgroundColor(0x23, 0xc7, 0xc9);
	static const sf::Color pausedColor(0x7f, 0x7f, 0x7f);
	m_game->window().clear(m_isPaused? pausedColor : backgroundColor);

	// set the view to center on the player
	// this is a side scroller so only worry about the x axis
	auto &playerPos = m_player->getComponent<CTransform>().pos;
	float centerX = std::max(m_game->window().getSize().x / 2.f, playerPos.x); // don't go left of window
	sf::View view = m_game->window().getView();
	view.setCenter(centerX, m_game->window().getSize().y - view.getCenter().y); // keep view at bottom
	m_game->window().setView(view);

	// draw all entities
	if (m_drawTextures)
	{
		for (auto e : m_entityManager.getEntities())
		{
			if (e->hasComponent<CAnimation>())
			{
				auto &tfm = e->getComponent<CTransform>();

				auto &anim = e->getComponent<CAnimation>().animation;
				anim.getSprite().setRotation(tfm.angle);
				anim.getSprite().setPosition(tfm.pos.x, tfm.pos.y);
				anim.getSprite().setScale(tfm.scale.x, tfm.scale.y);
				m_game->window().draw(anim.getSprite());
				
			}
		}
	}

	// draw all colliders
	if (m_drawColliders)
	{
		for (auto e : m_entityManager.getEntities())
		{
			if (e->hasComponent<CBoundingBox>())
			{
				auto &tfm = e->getComponent<CTransform>();
				auto &box = e->getComponent<CBoundingBox>();
				sf::RectangleShape rect(sf::Vector2f(box.size.x, box.size.y));
				rect.setOrigin(sf::Vector2f(box.halfSize.x, box.halfSize.y));
				rect.setFillColor(sf::Color(0, 0, 0, 0));
				rect.setOutlineColor(sf::Color(255, 255, 255));
				rect.setPosition(tfm.pos.x, tfm.pos.y);
				rect.setOutlineThickness(1.f);
				m_game->window().draw(rect);
			}
		}
	}

	// draw grids
	sf::VertexArray lines(sf::Lines);
	sf::Text gridText;
	gridText.setFont(Assets::getInstance().getFont("Arial"));
	gridText.setCharacterSize(10);

	if (m_drawGrid)
	{
		float left = view.getCenter().x - view.getSize().x / 2.f;
		float right = left + view.getSize().x;
		float top = view.getCenter().y - view.getSize().y / 2.f;
		float bottom = top + view.getSize().y;

		// align grid to grud size
		int nCols = static_cast<int>(view.getSize().x) / m_gridSize.x;
		int nRows = static_cast<int>(view.getSize().y) / m_gridSize.y;

		// row and col # of bot left
		const int ROW0 = 1080;
		int firstCol = static_cast<int>(left) / static_cast<int>(m_gridSize.x);
		int lastRow = static_cast<int>(bottom) / static_cast<int>(m_gridSize.y);

		lines.clear(); // clear the vertex array before adding new lines for the new frame

		// vertical lines
		for (int x{ 0 }; x <= nCols; ++x)
		{
			lines.append(sf::Vector2f((firstCol + x) * m_gridSize.x, top)); // set vertical line's top point
			lines.append(sf::Vector2f((firstCol + x) * m_gridSize.x, bottom)); // set vertical line's bottom point
			// (firstCol(=0) + x) * m_gridSize.x is the x coordinate to draw the vertical line
			// draw line from top(0) to bottom(1080), based on each Col's x coordinate
		}

		// horizontal lines
		for (int y{ 0 }; y <= nRows; ++y)
		{
			lines.append(sf::Vector2f(left, ROW0 - (lastRow - y) * m_gridSize.y)); // set horizontal line's left point
			lines.append(sf::Vector2f(right, ROW0 - (lastRow - y) * m_gridSize.y)); // set horizontal line's right point
			// ROW0 - (lastRow - y) * m_gridSize.y is the y coordinate to draw the horizontal line
			// draw line from left(0) to right(1920), based on each Row's y coordinate
		}

		// grid coordinates
		// (firstCol, LastRow) is the bottem left corner of the view
		// LastRow - nRows : (making bottom row to 0 in the coordinate represent, not lastRow's value)
		for (int x{ 0 }; x <= nCols; ++x)
		{
			for (int y{ 0 }; y <= nRows; ++y)
			{
				std::string label = std::string(
					"(" + std::to_string(firstCol + x) + ", " + std::to_string(lastRow - y) + " )"
				);
				gridText.setString(label);
				gridText.setPosition((firstCol + x) * m_gridSize.x, ROW0 - (lastRow - y + 1) * m_gridSize.y);
				m_game->window().draw(gridText);
			}
		}

		m_game->window().draw(lines);
	}

	//m_game->window().display();
}

void Scene_Wonder_Boy::checkPlayerState()
{
	auto &tfm = m_player->getComponent<CTransform>();
	auto &state = m_player->getComponent<CState>();

	// face the rightway
	if (std::abs(tfm.vel.x) > 0.1f)
	{
		tfm.scale.x = (tfm.vel.x > 0) ? abs(tfm.scale.x) : -abs(tfm.scale.x);
	}

	if (!state.test(CState::isGrounded))
	{
		//m_player->getComponent<CAnimation>().animation = Assets::getInstance().getAnimation("PlayerJump");
	}
	else
	{
		if (!state.test(CState::isRuuning)) // wasn't running
		{
			// change to running animation
			m_player->getComponent<CAnimation>().animation = Assets::getInstance().getAnimation("PlayerRun");
			state.set(CState::isRuuning);
		}
		else
		{
			
			state.unSet(CState::isRuuning);
		}
	}
}

Vec2 Scene_Wonder_Boy::gridToMidPixel(float gridX, float gridY, sPtrEntt entity)
{
	// (left, bot) of grix,gidy)

	// this is for side scroll, and based on window height being the same as world height
	// to be more generic and support scrolling up and down as well as left and right it
	// should be based on world size not window size
	float x = 0.f + gridX * m_gridSize.x;
	float y = 1080.f - gridY * m_gridSize.y;

	Vec2 spriteSize = entity->getComponent<CAnimation>().animation.getSize();

	return Vec2(x + spriteSize.x, y - spriteSize.y);
}


void Scene_Wonder_Boy::update(sf::Time dt)
{


	SoundPlayer::getInstance().removeStoppedSounds();
	m_entityManager.update();

	sMovement(dt);

	sAnimation();
	checkPlayerState();


}

void Scene_Wonder_Boy::sDoAction(const Command& command)
{
	// On Key Pressed
	if (command.type() == "START") {
		if (command.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
		else if (command.name() == "TOGGLE_COLLIDER") { m_drawColliders = !m_drawColliders; }
		else if (command.name() == "TOGGLE_GRID") { m_drawGrid = !m_drawGrid; }
		else if (command.name() == "PAUSE") { setPaused(!m_isPaused); }
		else if (command.name() == "QUIT") { onEnd(); }
		else if (command.name() == "LEFT")
		{
			m_player->getComponent<CInput>().left = true;
		}
		else if (command.name() == "RIGHT")
		{
			m_player->getComponent<CInput>().right = true;
		}
		else if (command.name() == "JUMP")
		{

		}
		else if (command.name() == "SHOOT")
		{
			//throwWeapon();
		}
	}

	// On Key Released
	else if (command.type() == "END") {
		if (command.name() == "LEFT")
		{
			m_player->getComponent<CInput>().left = false;
		}
		else if (command.name() == "RIGHT")
		{
			m_player->getComponent<CInput>().right = false;
		}
		if (command.name() == "JUMP")
		{

		}
		else if (command.name() == "SHOOT")
		{

		}
	}
}

void Scene_Wonder_Boy::spawnPlayer()
{
	m_player = m_entityManager.addEntity("player");
	m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("PlayerRun"), true);
	m_player->addComponent<CTransform>(gridToMidPixel(m_playerConfig.X, m_playerConfig.Y,  m_player), Vec2(0, 0), Vec2(3,3));
	m_player->addComponent<CInput>();
	m_player->addComponent<CBoundingBox>(Vec2(m_playerConfig.CW, m_playerConfig.CH));
	m_player->addComponent<CState>();

}


sf::FloatRect Scene_Wonder_Boy::getViewBounds()
{
	return sf::FloatRect();
}



void Scene_Wonder_Boy::loadLevel(const std::string& path)
{
	std::ifstream confFile(path);
	if (confFile.fail())
	{
		std::cerr << "Open file " << path << " failed\n";
		confFile.close();
		exit(1);
	}

	std::string token{ "" };
	confFile >> token;

	while (!confFile.eof())
	{
		

		if (token == "Player") {

			confFile >>
				m_playerConfig.X >>
				m_playerConfig.Y >>
				m_playerConfig.CW >>
				m_playerConfig.CH >>
				m_playerConfig.SPEED >>
				m_playerConfig.JUMP >>
				m_playerConfig.MAXSPEED >>
				m_playerConfig.GRAVITY >>
				m_playerConfig.WEAPON;
		}
		else if (token == "Tile")
		{
			std::string name;
			float gx, gy;
			confFile >> name >> gx >> gy;

			auto e = m_entityManager.addEntity("tile");
			e->addComponent<CAnimation>(Assets::getInstance().getAnimation(name), true);
			auto& tfm = e->addComponent<CTransform>(gridToMidPixel(gx, gy, e));
			tfm.scale = Vec2(3.1f, 3.1f);
			e->addComponent<CBoundingBox>(Vec2(32 * tfm.scale.x, 32 * tfm.scale.y));
			



		}
		else if (token == "#") 
		{
			; // ignore comments
			std::string tmp;
			std::getline(confFile, tmp);
			std::cout << "# " << tmp << "\n";
		}
		else 
		{
			std::cerr << "Unkown asset type: " << token << std::endl;
		}

		confFile >> token;
	}

	confFile.close();
}

void Scene_Wonder_Boy::registerActions()
{
	registerAction(sf::Keyboard::W, "JUMP");
	registerAction(sf::Keyboard::Up, "JUMP");
	registerAction(sf::Keyboard::A, "LEFT");
	registerAction(sf::Keyboard::Left, "LEFT");
	registerAction(sf::Keyboard::D, "RIGHT");
	registerAction(sf::Keyboard::Right, "RIGHT");
	registerAction(sf::Keyboard::Space, "SHOOT");

	registerAction(sf::Keyboard::P, "PAUSE");
	registerAction(sf::Keyboard::Escape, "QUIT");
	registerAction(sf::Keyboard::Q, "QUIT");

	registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE");
	registerAction(sf::Keyboard::C, "TOGGLE_COLLIDER");
	registerAction(sf::Keyboard::G, "TOGGLE_GRID");
}
