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

const Vec2 BB_SIZE(96, 96);
const Vec2 WEAPON_SPEED(30.f, -30.f);
const sf::Time THROW_TIMEPERFRAME = sf::seconds(0.1f);

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
	// player movement
	auto &pTfm = m_player->getComponent<CTransform>();
	auto &pState = m_player->getComponent<CState>();
	auto &pInput = m_player->getComponent<CInput>();
	auto &pAnim = m_player->getComponent<CAnimation>();
	// set canJump property with isGrounded state
	pInput.canJump = pState.test(CState::isGrounded);

	pTfm.vel.x = 0.f;

	if (pInput.left) 
		pTfm.vel.x -= 1;
	if (pInput.right) 
		pTfm.vel.x += 1; 
	if (abs(pTfm.vel.x) > 0)
		pTfm.scale.x = (pTfm.vel.x) > 0 ? 1.f : -1.f;

	// setting a running state should be rocated in checkPlayerState, not here	

	if (pInput.jump) 
	{
		pInput.jump = false;
		// set the jump state to false after getting the jump input, 
		// it prevents keep jumping when the jump key is pressed
		if (pState.test(CState::isGrounded))
		{
			pTfm.vel.y -= m_playerConfig.JUMP;
			pState.unSet(CState::isGrounded);
		}
	}

	if (pInput.canShoot)
	{
		if (pInput.shoot)
		{
			if (!(pState.test(CState::isThrowing))) {
 				pInput.shoot = false;
				pState.set(CState::isThrowing);
			}
 			else if (pState.test(CState::isThrowing))
			{
				pInput.shoot = false;
				// nothing happens
			}
		}
	}


	// gravity
	pTfm.vel.y += m_playerConfig.GRAVITY;
	pTfm.vel.x = pTfm.vel.x * m_playerConfig.SPEED;

	pTfm.pos += pTfm.vel;
	
	// move all entities
	for (auto e : m_entityManager.getEntities()) {
		auto& tfm = e->getComponent<CTransform>();
		tfm.prevPos = tfm.pos;
		if (e->hasComponent<CPhysics>()) {
			tfm.vel.y += e->getComponent<CPhysics>().gravity;
		}
		tfm.pos += tfm.vel;
	}

	(pTfm.vel.x == 0) ? pState.unSet(CState::isRunning) : pState.set(CState::isRunning);
	if (pState.test(CState::isFacingLeft))
	{
		if (pTfm.vel.x == 0)
			pState.set(CState::isFacingLeft);
		else
		{
			(pTfm.vel.x < 0) ? pState.set(CState::isFacingLeft) : pState.unSet(CState::isFacingLeft);
		}
	}
	else if (!pState.test(CState::isFacingLeft))
	{
		if (pTfm.vel.x == 0)
			pState.unSet(CState::isFacingLeft);
		else
		{
			(pTfm.vel.x < 0) ? pState.set(CState::isFacingLeft) : pState.unSet(CState::isFacingLeft);
		}
	}
}

void Scene_Wonder_Boy::sCollisions()
{
	auto &entities = m_entityManager.getEntities();
	auto &tiles = m_entityManager.getEntities("tile");
	auto &tilesBB = m_entityManager.getEntities("tilesBB");
	auto &halftileEBB = m_entityManager.getEntities("halftileEBB");
	auto &halftileSBB = m_entityManager.getEntities("halftileSBB");
	auto &weapons = m_entityManager.getEntities("weapon");

	auto &playerTfm = m_player->getComponent<CTransform>();
	auto &playerState = m_player->getComponent<CState>();
	auto &playerPhysics = m_player->getComponent<CPhysics>();

	for (auto& e : weapons)
	{
		// check if weapon is out of bounds
		auto& tfm = e->getComponent<CTransform>();
		if (tfm.pos.x < 0 || tfm.pos.y < 0 || tfm.pos.y > 1080)
		{
			e->destroy();
		}
		// check if weapon collides with tiles
		for (auto& t : tiles)
		{
			auto& tfm = t->getComponent<CTransform>();
			auto& box = t->getComponent<CBoundingBox>();
			auto overlap = Physics::getOverlap(e, t);
			if (overlap.x > 0 && overlap.y > 0)
			{
				e->destroy();
			}
		}
		// check if weapon collides with tilesBB
		for (auto& t : tilesBB)
		{
			auto& tfm = t->getComponent<CTransform>();
			auto& box = t->getComponent<CBoundingBox>();
			auto overlap = Physics::getOverlap(e, t);
			if (overlap.x > 0 && overlap.y > 0)
			{
				e->destroy();
			}
		}
		// check if weapon collides with enmemies
		// Todo* 
	}

	if (m_player->hasComponent<CBoundingBox>())
	{

		for (auto &e : tiles)
		{
			auto& tfm = e->getComponent<CTransform>();
			auto& box = e->getComponent<CBoundingBox>();

			auto overlap = Physics::getOverlap(m_player, e);
			auto preoverlap = Physics::getPreviousOverlap(m_player, e);

			if (e->hasComponent<CBoundingBox>())
			{
				
				if (overlap.x > 0 && overlap.y > 0) 
				{
					if (preoverlap.y > 0) 
					{
						playerTfm.pos.y -= overlap.y; // move up
						playerTfm.vel.y = 0; // stop falling
						playerState.set(CState::isGrounded); // set grounded state
					}
				}
			}
		}
		for (auto &e: tilesBB)
		{
			auto& tfm = e->getComponent<CTransform>();
			auto& box = e->getComponent<CBoundingBox>();

			auto overlap = Physics::getOverlap(m_player, e);
			auto preoverlap = Physics::getPreviousOverlap(m_player, e);
			
			if (e->hasComponent<CBoundingBox>())
			{
				if (overlap.x > 0 && overlap.y > 0)
				{
					if (preoverlap.y > 0)
					{ // if was above
						playerTfm.pos.y -= overlap.y; // move up
						playerTfm.vel.y = 0; // stop falling
						playerState.set(CState::isGrounded); // set grounded state
					}
					if (preoverlap.y < 0)
					{ // if was below
						playerTfm.pos.y -= overlap.y; // move up
						playerTfm.vel.y = 0; // stop falling
						playerState.set(CState::isGrounded); // set grounded state
					}
				}
			}
		}
		for (auto& e : halftileEBB)
		{

		}
		for (auto& e : halftileSBB)
		{

		}

	}

}

void Scene_Wonder_Boy::sAnimation(sf::Time dt)
{
	for (auto e : m_entityManager.getEntities())
	{
		auto &anim = e->getComponent<CAnimation>();
		if (anim.has) {
			anim.animation.update(dt);
			/*if (e->getTag() != "player")
				if (anim.animation.hasEnded()) { e->destroy(); }*/
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
	static const sf::Color backgroundColor(0, 181, 255);
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

void Scene_Wonder_Boy::checkPlayerState() // check player state and change animation accordingly
{
	auto &pTfm = m_player->getComponent<CTransform>();
	auto &pState = m_player->getComponent<CState>();
	auto &pAnim = m_player->getComponent<CAnimation>().animation;
	auto &pInput = m_player->getComponent<CInput>();
	
		/*
		isGrounded = 1,        
		isFacingLeft = 1 << 1,   
		isRunning = 1 << 2,   
		onSkate = 1 << 3,   
		isAlive = 1 << 4,   
		isThrowing = 1 << 5,  
		*/
	
	if (pState.test(CState::isAlive) && pState.test(CState::isThrowing) && !pAnim.hasEnded())
	{
		if (pAnim.getName() != "tt_axe") {
			pAnim = Assets::getInstance().getAnimation("tt_axe");
			pAnim.setTFM(THROW_TIMEPERFRAME);
		}	
	}
	else if (pState.test(CState::isAlive) && pState.test(CState::isThrowing) && pAnim.hasEnded())
	{
		pState.unSet(CState::isThrowing);
	}
	else if (pState.test(CState::isAlive) && !pState.test(CState::isThrowing) && pState.test(CState::isRunning))
	{
		if (pAnim.getName() != "tt_run")
			pAnim = Assets::getInstance().getAnimation("tt_run");
	}
	else if (pState.test(CState::isAlive) && !pState.test(CState::isThrowing) && !pState.test(CState::isRunning))
	{
		if (pAnim.getName() != "tt_stand")
			pAnim = Assets::getInstance().getAnimation("tt_stand");
	}
	else if (!pState.test(CState::isAlive))
	{
		pAnim = Assets::getInstance().getAnimation("tt_fall");
	}

}

void Scene_Wonder_Boy::spawnBullet(std::shared_ptr<Entity> e) {
	auto pTfm = e->getComponent<CTransform>();
	auto pState = e->getComponent<CState>();

	if (pTfm.has) {
		auto bullet = m_entityManager.addEntity("bullet");
		bullet->addComponent<CAnimation>(Assets::getInstance().getAnimation(m_playerConfig.WEAPON), true);
		std::cout << bullet->getComponent<CAnimation>().animation.getTFM().asSeconds() << std::endl;
		bullet->addComponent<CTransform>(pTfm.pos);
		bullet->addComponent<CBoundingBox>(Assets::getInstance().getAnimation(m_playerConfig.WEAPON).getSize());
		bullet->addComponent<CLifespan>(50);
		bullet->getComponent<CTransform>().vel.x = WEAPON_SPEED.x * (e->getComponent<CState>().test(CState::isFacingLeft) ? -1 : 1);
		bullet->getComponent<CTransform>().vel.y = WEAPON_SPEED.y;
		bullet->addComponent<CPhysics>(3, 0, 0, 0);
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

	return Vec2(x + spriteSize.x/ 2.f, y - spriteSize.y / 2.f);
}

void Scene_Wonder_Boy::sLifeSpan()
{
	for (auto e : m_entityManager.getEntities("bullet"))
	{
		auto &lifespan = e->getComponent<CLifespan>();
		if (lifespan.has)
		{
			lifespan.remainingTime -= 1;
			if (lifespan.remainingTime < 0)
			{
				e->getComponent<CLifespan>().has = false;
				e->getComponent<CTransform>().vel.x *= 0.1f;
			}
		}
	}
}


void Scene_Wonder_Boy::update(sf::Time dt)
{


	SoundPlayer::getInstance().removeStoppedSounds();
	m_entityManager.update();


	sMovement(dt);
	sCollisions();

	sAnimation(dt);

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
			m_player->getComponent<CInput>().jump = true;
		}
		else if (command.name() == "SHOOT")
		{
			//throwWeapon();
			if (m_player->getComponent<CInput>().canShoot) {
				spawnBullet(m_player);
				m_player->getComponent<CInput>().shoot = true;
			}
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
	m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("tt_stand"), true);
	m_player->addComponent<CTransform>(gridToMidPixel(m_playerConfig.X, m_playerConfig.Y,  m_player), Vec2(0, 0), Vec2(1,1));
	m_player->addComponent<CState>().unSet(CState::isRunning);
	m_player->addComponent<CInput>();
	m_player->addComponent<CBoundingBox>(Vec2(m_playerConfig.CW, m_playerConfig.CH));
	m_player->addComponent<CState>().set(CState::isAlive);
	m_player->addComponent<CPhysics>(m_playerConfig.GRAVITY, m_playerConfig.MAXSPEED, m_playerConfig.SPEED, m_playerConfig.JUMP);

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
			e->addComponent<CBoundingBox>(BB_SIZE);
			

		}
		else if (token == "Uphill" || token == "Downhill")
		{
			std::string name;
			float gx, gy;
			size_t sN =4;
			confFile >> name >> gx >> gy;

			auto e = m_entityManager.addEntity("tilesAnim");
			e->addComponent<CAnimation>(Assets::getInstance().getAnimation(name), true);
			auto& tfm = e->addComponent<CTransform>(gridToMidPixel(gx, gy, e));

			float width = GRID_SIZE * sN;
			size_t wantDivide = 4;
			size_t num = wantDivide * sN;
			size_t unitSize = GRID_SIZE / wantDivide;

			float originX = tfm.pos.x;
			float originY = tfm.pos.y;

			float startX = originX - (GRID_SIZE * sN / 2);
			float startY = originY + (GRID_SIZE / 2);
			float changeX = startX - (unitSize / 2);
			float changeY = startY;
			for (int i{ 0 }; i < sN; i++)
			{
				for (int j{ 0 }; j < wantDivide; j++)
				{
					auto e = m_entityManager.addEntity("tilesBB");
					changeX += unitSize;
					e->addComponent<CTransform>(Vec2(changeX, changeY));
					e->addComponent<CBoundingBox>(Vec2(static_cast<float>(unitSize), static_cast<float>(unitSize)));
				}
				changeY -= unitSize;
			}

		}
		else if (token == "HalftileE") // add 1 half tile with 2 half BBs on vertically in the left side
		{
			std::string name;
			float gx, gy;
			confFile >> name >> gx >> gy;

			auto e = m_entityManager.addEntity("halftileE");
			e->addComponent<CAnimation>(Assets::getInstance().getAnimation(name), true);
			e->addComponent<CTransform>(gridToMidPixel(gx, gy, e));


			auto halfNGrid = GRID_SIZE / 2;
			auto eb1 = m_entityManager.addEntity("halftileEBB");
			auto &tfm1 = eb1->addComponent<CTransform>(gridToMidPixel(gx - 0.25, gy + 0.25, e));
			eb1->addComponent<CBoundingBox>(Vec2(GRID_SIZE, GRID_SIZE) * 0.5);
			auto eb2 = m_entityManager.addEntity("halftileEBB");
			auto& tfm2 = eb2->addComponent<CTransform>(gridToMidPixel(gx - 0.25, gy - 0.25, e));
			eb2->addComponent<CBoundingBox>(Vec2(GRID_SIZE, GRID_SIZE) * 0.5);
		}
		else if (token == "HalftileS") // add 1 half tile with 2 half BBs on vertically in the right side
		{
			std::string name;
			float gx, gy;
			confFile >> name >> gx >> gy;

			auto e = m_entityManager.addEntity("halftileS");
			e->addComponent<CAnimation>(Assets::getInstance().getAnimation(name), true);
			e->addComponent<CTransform>(gridToMidPixel(gx, gy, e));


			auto halfNGrid = GRID_SIZE / 2;
			auto eb1 = m_entityManager.addEntity("halftileSBB");
			auto& tfm1 = eb1->addComponent<CTransform>(gridToMidPixel(gx + 0.25, gy + 0.25, e));
			eb1->addComponent<CBoundingBox>(Vec2(GRID_SIZE, GRID_SIZE) * 0.5);
			auto eb2 = m_entityManager.addEntity("halftileSBB");
			auto& tfm2 = eb2->addComponent<CTransform>(gridToMidPixel(gx + 0.25, gy - 0.25, e));
			eb2->addComponent<CBoundingBox>(Vec2(GRID_SIZE, GRID_SIZE) * 0.5);
		}
		else if (token == "Deco")
		{
			std::string name;
			float gx, gy;
			confFile >> name >> gx >> gy;

			auto e = m_entityManager.addEntity("deco");
			e->addComponent<CAnimation>(Assets::getInstance().getAnimation(name), true);
			auto& tfm = e->addComponent<CTransform>(gridToMidPixel(gx, gy, e));
			e->addComponent<CBoundingBox>();


		}
		else if (token == "Item")
		{
			std::string name;
			float gx, gy;
			confFile >> name >> gx >> gy;

			auto e = m_entityManager.addEntity("item");
			auto IAnim = e->addComponent<CAnimation>(Assets::getInstance().getAnimation(name), true);

			auto& tfm = e->addComponent<CTransform>(gridToMidPixel(gx, gy, e));
			e->addComponent<CBoundingBox>();
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
