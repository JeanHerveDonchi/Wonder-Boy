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
const float RESPAWN_DEPTH = 1700.f;
const Vec2 SPAWN_POS1(2, 10);
const Vec2 SPAWN_POS2(2, 10);
const Vec2 SPAWN_POS3(8, 10);
const Vec2 SPAWN_POS4(11, 10);
const Vec2 GOAL_POS(14, 10);
const float ENEMY_D_RANGE = 750.f;

Scene_Wonder_Boy::Scene_Wonder_Boy(GameEngine* gameEngine, const std::string& levelPath)
	: Scene(gameEngine)
{
	init(levelPath);
}


void Scene_Wonder_Boy::init(const std::string& levelPath)
{
	m_game->window().setView(m_game->window().getDefaultView());
	m_worldView = m_game->window().getDefaultView();

	loadLevel(levelPath);
	registerActions();
	
	spawnPlayer();

	MusicPlayer::getInstance().play("level01");
	MusicPlayer::getInstance().setVolume(50);

	m_clock.restart();
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

	if (pInput.has)
	{
		pTfm.vel.x = 0;

		if (pInput.left)
			pTfm.vel.x = -m_playerConfig.SPEED;
		if (pInput.right)
			pTfm.vel.x = m_playerConfig.SPEED;
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
					spawnBullet(m_player);
				}
				else if (pState.test(CState::isThrowing))
				{
					pInput.shoot = false;
					// nothing happens
				}
			}
		}
	}

	// tripping
	if (pState.test(CState::isTripping))
	{	
			pInput.canJump = false;
			const float TRIPPING_SPEED = 5.f;
			pTfm.vel.x = (pTfm.prevPos.x - pTfm.pos.x) < 0 ? TRIPPING_SPEED : -TRIPPING_SPEED;
			pTfm.vel.y = -3;
	}
	else if (!pState.test(CState::isTripping))
	{
		pInput.canJump = true;
	}

	if (m_player->getComponent<CTransform>().pos.x > 39100)
	{
		onEnd();
	}

	// move all entities
	for (auto e : m_entityManager.getEntities()) {
		auto& tfm = e->getComponent<CTransform>();
		tfm.prevPos = tfm.pos;
		if (e->hasComponent<CPhysics>()) {
			tfm.vel.y += e->getComponent<CPhysics>().gravity;
		}
		if (e->hasComponent<CAI>() && e->getComponent<CAI>().detectPlayer)
		{
			auto &ai = e->getComponent<CAI>();

			if (!e->getComponent<CAI>().moveVertically)
				tfm.vel = Vec2(-ai.velX, tfm.vel.y);
			else
			{
				if (tfm.pos.y > ai.maxY || tfm.pos.y < ai.minY)
					ai.velY = -ai.velY;
				tfm.vel = Vec2(-ai.velX, ai.velY);
			}
		}
		tfm.pos += tfm.vel;
	}

	// setting isRunning and isFacingLeft states
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
	auto& entities = m_entityManager.getEntities();
	auto& tiles = m_entityManager.getEntities("tile");
	auto& tilesBB = m_entityManager.getEntities("tilesBB");
	auto& halftileEBB = m_entityManager.getEntities("halftileEBB");
	auto& halftileSBB = m_entityManager.getEntities("halftileSBB");
	auto& bullets = m_entityManager.getEntities("bullet");
	auto& items = m_entityManager.getEntities("item");
	auto& enemies = m_entityManager.getEntities("enemy");
	auto& obstacles = m_entityManager.getEntities("obstacle");
	auto& helpers = m_entityManager.getEntities("helper");
	auto& rollingRocks = m_entityManager.getEntities("rollingRock");
	auto& skateBoards = m_entityManager.getEntities("skateBoard");

 	auto& pTfm = m_player->getComponent<CTransform>();
	auto& pState = m_player->getComponent<CState>();
	auto& pPhysics = m_player->getComponent<CPhysics>();
	auto& pInput = m_player->getComponent<CInput>();

	for (auto& e : bullets)
	{
		// check if weapon is out of bounds
		auto& tfm = e->getComponent<CTransform>();
		if (tfm.pos.x < 0 || tfm.pos.y > 1080)
		{
			e->destroy();
		}
		// check if weapon collides with enmemies
		for (auto& enemy : enemies)
		{
			auto& tfm = enemy->getComponent<CTransform>();
			auto& box = enemy->getComponent<CBoundingBox>();
			auto overlap = Physics::getOverlap(e, enemy);
			if (overlap.x > 0 && overlap.y > 0)
			{
				e->destroy();
				enemy->destroy();
			}
		}
	}

	if (m_player->hasComponent<CBoundingBox>())
	{
		for (auto& e : skateBoards) {
			auto& tfm = e->getComponent<CTransform>();
			auto& box = e->getComponent<CBoundingBox>();

			auto overlap = Physics::getOverlap(m_player, e);
			auto preoverlap = Physics::getPreviousOverlap(m_player, e);

			if (e->hasComponent<CBoundingBox>())
			{
				if (overlap.x > 0 && overlap.y > 0)
				{
					m_player->getComponent<CState>().set(CState::onSkate);
					e->destroy();
				}
			}
		}
		for (auto& e : tiles)
		{
			auto& tfm = e->getComponent<CTransform>();
			auto& box = e->getComponent<CBoundingBox>();

			auto overlap = Physics::getOverlap(m_player, e);
			auto preoverlap = Physics::getPreviousOverlap(m_player, e);

			if (e->hasComponent<CBoundingBox>())
			{
				
				if (overlap.x > 0 && overlap.y > 0) 
				{
					// collision is in the y direction
					if (preoverlap.x > 0) {
						if (pTfm.prevPos.y < tfm.prevPos.y) {
							// player standing on something isGrounded
							pTfm.pos.y -= overlap.y;
							pState.set(CState::isGrounded);
						}
						pTfm.vel.y = 0.f;
					}
					// collision is in the x direction
					if (preoverlap.y > 0) {
						pTfm.pos.x -= overlap.x;
						pTfm.vel.x = 0.f;
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
					// collision is in the y direction
					if (preoverlap.x > 0) {
						if (pTfm.prevPos.y < tfm.prevPos.y) {
							// player standing on something isGrounded
							pTfm.pos.y -= overlap.y;
							pState.set(CState::isGrounded);
						}
						pTfm.vel.y = 0.f;
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
		for (auto& e : items)
		{
			auto& tfm = e->getComponent<CTransform>();
			auto& box = e->getComponent<CBoundingBox>();

			auto overlap = Physics::getOverlap(m_player, e);
			auto preoverlap = Physics::getPreviousOverlap(m_player, e);

			if (e->hasComponent<CBoundingBox>())
			{
				if (overlap.x > 0 && overlap.y > 0)
				{
					SoundPlayer::getInstance().play("item");

					e->removeComponent<CBoundingBox>();
					std::string aName = e->getComponent<CAnimation>().animation.getName();
					auto& anim = e->getComponent<CAnimation>().animation;
					auto& aTfm = e->getComponent<CTransform>();
					if (aName == "apple")
					{
						anim = Assets::getInstance().getAnimation("50");
						aTfm.scale = Vec2(2.f, 2.f);
						m_score += 50;
						m_earnedTime += 1;
					}
					else if (aName == "tomato")
					{
						anim = Assets::getInstance().getAnimation("100");
						aTfm.scale = Vec2(2.f, 2.f);
						m_score += 100; 
						m_earnedTime += 2;
					}
					else if (aName == "banana")
					{
						anim = Assets::getInstance().getAnimation("200");
						aTfm.scale = Vec2(2.f, 2.f);
						m_score += 200;
						m_earnedTime += 2;
					}
					else if (aName == "milk")
					{
						anim = Assets::getInstance().getAnimation("500");
						aTfm.scale = Vec2(2.f, 2.f);
						m_score += 500;
						m_earnedTime += 3;
					}
					else if (aName == "candy")
					{
						anim = Assets::getInstance().getAnimation("1000");
						aTfm.scale = Vec2(2.f, 2.f);
						m_score += 1000;
						m_earnedTime += 4;
					}
					else if (aName == "axe")
					{
						m_player->getComponent<CInput>().canShoot = true;
						e->destroy();
					}
					else
					{
						e->destroy();
					}
					
				}			
			}
		}
		for (auto& e : enemies)
		{
			auto& tfm = e->getComponent<CTransform>();
			auto& box = e->getComponent<CBoundingBox>();


			auto overlap = Physics::getOverlap(m_player, e);
			auto preoverlap = Physics::getPreviousOverlap(m_player, e);

			if (box.has)
			{
				if (overlap.x > 0 && overlap.y > 0)
				{
					pState.unSet(CState::isAlive);
					pTfm.vel.y = -12;
					MusicPlayer::getInstance().stop();
					SoundPlayer::getInstance().stop();
					SoundPlayer::getInstance().play("die");
				}
			}

		}
		for (auto& e : obstacles)
		{
			auto& tfm = e->getComponent<CTransform>();
			auto& box = e->getComponent<CBoundingBox>();
			auto& anim = e->getComponent<CAnimation>().animation;

			auto overlap = Physics::getOverlap(m_player, e);
			auto preoverlap = Physics::getPreviousOverlap(m_player, e);

			if (box.has)
			{
				if (anim.getName() == "rock")
				{
					if (overlap.x > 0 && overlap.y > 0)
					{
						if (!pState.test(CState::isTripping))
						{

							pState.set(CState::isTripping);

							m_earnedTime -= 3;

							SoundPlayer::getInstance().play("trip");
						}
					}
				}
				else if(anim.getName() == "fire")
				{
					if (overlap.x > 0 && overlap.y > 0)
					{
						pState.unSet(CState::isAlive);
						pState.set(CState::isBurned);
						pTfm.vel.y = -12;
						MusicPlayer::getInstance().stop();
						SoundPlayer::getInstance().stop();
						SoundPlayer::getInstance().play("die");
					}
				}
			}
		}
		
		for (auto& h : helpers)
		{
			auto overlap = Physics::getOverlap(m_player, h);
			auto preoverlap = Physics::getPreviousOverlap(m_player, h);
			if (overlap.x > 0 && overlap.y > 0) {
				// collision is in the y direction
				if (preoverlap.x > 0) {
					if (pTfm.prevPos.y < h->getComponent<CTransform>().prevPos.y) {
						
						// player standing on something isGrounded
						pTfm.pos.y -= overlap.y;
						pState.set(CState::isGrounded);
								
					}
					else
					{
						pTfm.pos.y += overlap.y;
					}
					pTfm.vel.y = 0.f;

					if (h->getComponent<CAnimation>().animation.getName() == "springOBefore")
					{
						pTfm.pos.y -= overlap.y;
						pState.unSet(CState::isGrounded);
						pTfm.vel.y = -50;
						h->getComponent<CAnimation>().animation = Assets::getInstance().getAnimation("springO");
					}
					else if (h->getComponent<CAnimation>().animation.getName() == "springO")
					{
						pTfm.pos.y -= overlap.y;
						pState.unSet(CState::isGrounded);
						pTfm.vel.y = -50;
					}
				}
			}
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
			if (e->getTag() != "player")
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
	static const sf::Color backgroundColor(0, 181, 255);
	m_game->window().clear(backgroundColor);
	//static const sf::Color pausedColor(0x7f, 0x7f, 0x7f);
	//m_game->window().clear(m_isPaused? pausedColor : backgroundColor);

	// set the view to center on the player
	// this is a side scroller so only worry about the x axis
	auto &playerPos = m_player->getComponent<CTransform>().pos;
	float centerX = std::max(m_game->window().getSize().x / 2.f, playerPos.x); // don't go left of window
	/*sf::View view = m_game->window().getView();*/
	float centerY = m_game->window().getSize().y / 2.f;
	// 250 - 275
	/*if (playerPos.x > 26400)
	{
		centerY = 100.f;
	}*/
	if (playerPos.x > 250 * 96 && playerPos.x < 273 * 96)
	{
		centerY -= (playerPos.x - 250 * 96) * 0.26 + 6;
		m_worldView.setCenter(centerX, centerY);
	}
	else
		m_worldView.setCenter(centerX, m_worldView.getCenter().y);
	
	m_game->window().setView(m_worldView);
	

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
		float left = m_worldView.getCenter().x - m_worldView.getSize().x / 2.f;
		float right = left + m_worldView.getSize().x;
		float top = m_worldView.getCenter().y - m_worldView.getSize().y / 2.f;
		float bottom = top + m_worldView.getSize().y;

		// align grid to grud size
		int nCols = static_cast<int>(m_worldView.getSize().x) / m_gridSize.x;
		int nRows = static_cast<int>(m_worldView.getSize().y) / m_gridSize.y + 5;

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


	drawHUD();

	//m_game->window().display();

	// * set HUD related to m_worldView, not entire window.
}

void Scene_Wonder_Boy::checkPlayerState() // check player state and change animation accordingly
{
	if (m_lifeCount <= 0)
	{
		onEnd();
	}

	auto &pTfm = m_player->getComponent<CTransform>();
	auto &pState = m_player->getComponent<CState>();
	auto &pAnim = m_player->getComponent<CAnimation>().animation;
	auto &pInput = m_player->getComponent<CInput>();
	auto &pBB = m_player->getComponent<CBoundingBox>();
	
		/*
		isGrounded = 1,        
		isFacingLeft = 1 << 1,   
		isRunning = 1 << 2,   
		onSkate = 1 << 3,   
		isAlive = 1 << 4,   
		isThrowing = 1 << 5,  
		*/

	
	
	if (pState.test(CState::isAlive))
	{
		if (m_remainingTime < 0.f)
		{
			m_player->getComponent<CState>().unSet(CState::isAlive);
			pTfm.vel.y = -12;

			MusicPlayer::getInstance().stop();
			SoundPlayer::getInstance().stop();
			SoundPlayer::getInstance().play("die");
		}
		if (pState.test(CState::isTripping))
		{
			if (!pAnim.hasEnded())
			{
				if (pAnim.getName() != "tt_trip") {
					pAnim = Assets::getInstance().getAnimation("tt_trip");
				}
			}
			else
			{
				pState.unSet(CState::isTripping);
			}
		}
		else if (pState.test(CState::onSkate))
		{
			if (pAnim.getName() != "tt_skate_run") {
				pAnim = Assets::getInstance().getAnimation("tt_skate_run");
			}
		}
		else if (pState.test(CState::isThrowing))
		{
			if (!pAnim.hasEnded())
			{
				if (pAnim.getName() != "tt_axe") {
					pAnim = Assets::getInstance().getAnimation("tt_axe");
				}
			}
			else
			{
				pState.unSet(CState::isThrowing);
			}
		}
		else 
		{
			if (pState.test(CState::isRunning))
			{
				if (pAnim.getName() != "tt_run")
					pAnim = Assets::getInstance().getAnimation("tt_run");
			}
			else
			{
				if (pAnim.getName() != "tt_stand")
					pAnim = Assets::getInstance().getAnimation("tt_stand");
			}
		}

	}
	else if (!pState.test(CState::isAlive)) // == else
	{

		pBB.has = false;
		pInput.has = false;
		pTfm.vel.x = 0;
		m_player->getComponent<CPhysics>().gravity = 0.3;
		m_player->getComponent<CState>().unSet(CState::isGrounded);


		if (pState.test(CState::isTripping))
		{
			pState.unSet(CState::isTripping);
			pTfm.vel.y = -12;
		}

		if (!pState.test(CState::isBurned) && pAnim.getName() != "tt_fall")
		{
			pAnim = Assets::getInstance().getAnimation("tt_fall");
		}
		else if (pState.test(CState::isBurned) && pAnim.getName() != "tt_fire")
		{
			pAnim = Assets::getInstance().getAnimation("tt_fire");
		}
	}
	if (pTfm.pos.y > RESPAWN_DEPTH)
	{
		auto& enemies = m_entityManager.getEntities("enemy");
		auto& items = m_entityManager.getEntities("item");

		for (auto& e : m_entityManager.getEntities())
			e->destroy();
		loadLevel("../Assets/level1.txt");
		m_lifeCount--;
		Vec2 checkedSpawnPos = Vec2(5, 7);
		if (pTfm.pos.x > 274 * 96)
		{
			checkedSpawnPos = Vec2(274, 7);
		}
		else if (pTfm.pos.x > 182 * 96)
		{
			checkedSpawnPos = Vec2(182, 7);
		}
		else if (pTfm.pos.x > 100 * 96)
		{
			checkedSpawnPos = Vec2(100, 7);
		}
		spawnPlayer(checkedSpawnPos); // replace this parameter to designated respawn points
		MusicPlayer::getInstance().play("level01");
	}
}

void Scene_Wonder_Boy::spawnBullet(std::shared_ptr<Entity> e) {
	auto pTfm = e->getComponent<CTransform>();
	auto pState = e->getComponent<CState>();

	if (pTfm.has) {
		auto bullet = m_entityManager.addEntity("bullet");
		bullet->addComponent<CAnimation>(Assets::getInstance().getAnimation(m_playerConfig.WEAPON), true);
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

void Scene_Wonder_Boy::sEnemyAI()
{
	auto enemies = m_entityManager.getEntities("enemy");
	auto pState = m_player->getComponent<CState>();
	auto pTfm = m_player->getComponent<CTransform>();
	for (auto e : enemies)
	{
		if (pState.test(CState::isAlive) && e->getComponent<CAI>().has)
		{
			if (e->getComponent<CTransform>().pos.x - pTfm.pos.x < ENEMY_D_RANGE)
			{
				e->getComponent<CAI>().detectPlayer = true;
			}
		}
	}
}


void Scene_Wonder_Boy::update(sf::Time dt)
{


	SoundPlayer::getInstance().removeStoppedSounds();
	m_entityManager.update();

	if (m_isPaused)
		return;

	sMovement(dt);
	sEnemyAI();
	sCollisions();

	sAnimation(dt);

	checkPlayerState();

	if (m_score > m_highScore)
	{
		m_highScore = m_score;
	}

	int elapsed = m_clock.getElapsedTime().asSeconds() /2.f;
	
	m_remainingTime = m_timeCount + m_earnedTime - elapsed;
	if (m_remainingTime > m_timeCount)
	{
		m_earnedTime -= (m_remainingTime - m_timeCount);
		m_remainingTime = m_timeCount;
	}
	else
	{
		m_remainingTime = m_timeCount - elapsed + m_earnedTime;
	}
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

void Scene_Wonder_Boy::spawnPlayer(Vec2 spawnPos)
{
	auto& pInput = m_player->getComponent<CInput>();
	auto& pBB = m_player->getComponent<CBoundingBox>();
	auto& pTfm = m_player->getComponent<CTransform>();
	
	m_player = m_entityManager.addEntity("player");
	m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("tt_stand"), true);
	m_player->addComponent<CTransform>(gridToMidPixel(spawnPos.x, spawnPos.y,  m_player), Vec2(0, 0), Vec2(1,1));
	m_player->addComponent<CState>().unSet(CState::isRunning);
	m_player->addComponent<CInput>();
	m_player->addComponent<CBoundingBox>(Vec2(m_playerConfig.CW, m_playerConfig.CH));
	m_player->addComponent<CState>().set(CState::isAlive);
	m_player->addComponent<CPhysics>(m_playerConfig.GRAVITY, m_playerConfig.MAXSPEED, m_playerConfig.SPEED, m_playerConfig.JUMP);

	m_earnedTime = 0;
	m_clock.restart();

	
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
		else if (token == "Tina")
		{
			std::string name;
			float gx, gy;
			confFile >> name >> gx >> gy;

			auto tina = m_entityManager.addEntity("tina");	
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
			float changeY = startY - 13;
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

			if (name == "start1Wood" || name == "start2Wood" || name == "start3Wood" || name == "start4Wood" || name == "goalWood")
			{				
				tfm.scale = Vec2(2, 2);
				tfm.pos.y -= 25.f;
			}
			
		}
		else if (token == "Item")
		{
			std::string name;
			float gx, gy;
			confFile >> name >> gx >> gy;

			auto e = m_entityManager.addEntity("item");
			auto& IAnim = e->addComponent<CAnimation>(Assets::getInstance().getAnimation(name), true);
			

			auto& tfm = e->addComponent<CTransform>(gridToMidPixel(gx, gy, e));
			e->addComponent<CBoundingBox>(IAnim.animation.getSize());
			
		}
		else if (token == "Enemy")
		{
			std::string name;
			float gx, gy;
			confFile >> name >> gx >> gy;

			auto e = m_entityManager.addEntity("enemy");
			auto& EAnim = e->addComponent<CAnimation>(Assets::getInstance().getAnimation(name), true);
			auto& tfm = e->addComponent<CTransform>(gridToMidPixel(gx, gy, e));
			tfm.scale = Vec2(2, 2);
			tfm.pos.y -= 15.f;
			e->addComponent<CBoundingBox>(EAnim.animation.getSize() * 1.2f);
			
			if (name == "bee")
			{
				auto& ai = e->addComponent<CAI>(10, true, 5, 850, 650);
			}
			else if (name == "snake")
			{
				tfm.pos.y -= 15.f;
			}
		}
		else if (token == "Obst")
		{
			std::string name;
			float gx, gy;
			confFile >> name >> gx >> gy;

			auto e = m_entityManager.addEntity("obstacle");
			auto& EAnim = e->addComponent<CAnimation>(Assets::getInstance().getAnimation(name), true);
			auto& tfm = e->addComponent<CTransform>(gridToMidPixel(gx, gy, e));
			e->addComponent<CBoundingBox>(Vec2(EAnim.animation.getSize().x * 0.7f, EAnim.animation.getSize().x * 0.8f));
		}
		else if (token == "Helper")
		{
			std::string name;
			float gx, gy;
			confFile >> name >> gx >> gy;

			auto e = m_entityManager.addEntity("helper");
			auto& EAnim = e->addComponent<CAnimation>(Assets::getInstance().getAnimation(name), true);
			auto& tfm = e->addComponent<CTransform>(gridToMidPixel(gx, gy, e));
			if (EAnim.animation.getName() == "springOBefore")
			{
				tfm.scale = Vec2(1.5f, 1.5f);
				e->addComponent<CBoundingBox>(EAnim.animation.getSize() * 1.5f);
			}
			else
				e->addComponent<CBoundingBox>(EAnim.animation.getSize());
		}
		else if (token == "RollingRock")
		{
				std::string name;
				float gx, gy;
				confFile >> name >> gx >> gy;

				auto e = m_entityManager.addEntity("rollingRock");
				auto& EAnim = e->addComponent<CAnimation>(Assets::getInstance().getAnimation(name), true);
				auto& tfm = e->addComponent<CTransform>(gridToMidPixel(gx, gy, e));
				e->addComponent<CBoundingBox>(EAnim.animation.getSize());
				e->addComponent<CPhysics>(1, 0, 0, 0);
				e->addComponent<CAI>(10);
		}
		else if (token == "SkateBoard") {
			std::string name;
			float gx, gy;
			confFile >> name >> gx >> gy;

			auto e = m_entityManager.addEntity("skateBoard");
			auto& EAnim = e->addComponent<CAnimation>(Assets::getInstance().getAnimation(name), true);
			auto& tfm = e->addComponent<CTransform>(gridToMidPixel(gx, gy, e));
			e->addComponent<CBoundingBox>(EAnim.animation.getSize());
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

void Scene_Wonder_Boy::drawHUD()
{
	auto& playerPos = m_player->getComponent<CTransform>().pos;
	float centerX = std::max(m_game->window().getSize().x / 2.f, playerPos.x); // don't go left of window
	float centerY = m_game->window().getSize().y / 2.f;

	float scoreX = std::max(m_worldView.getSize().x / 2.f - 450.f, playerPos.x - 450.f);
	float scoreY = m_worldView.getCenter().y - 530.f;
	float hScoreX = std::max(m_worldView.getSize().x / 2.f - 200.f, playerPos.x - 200.f);
	float hScoreY = m_worldView.getCenter().y - 530.f;
	float timerX = std::max(m_worldView.getSize().x / 2.f + 200.f, playerPos.x + 200.f);
	float timerY = m_worldView.getCenter().y - 530.f;
	float lifeX = std::max(m_worldView.getSize().x / 2.f - 800.f, playerPos.x - 800.f);
	float lifeY = m_worldView.getCenter().y - 510.f;

	// Draw Score
	sf::Text lblScore("Score", Assets::getInstance().getFont("Arial"), 40);
	lblScore.setPosition(scoreX, scoreY);
	sf::Text score(std::to_string(m_score), Assets::getInstance().getFont("Arial"), 40);
	score.setPosition(scoreX, scoreY + 50.f);
	score.setFillColor(sf::Color::Magenta);

	// Draw High Score
	sf::Text lblHighScore("High Score", Assets::getInstance().getFont("Arial"), 40);
	lblHighScore.setPosition(hScoreX, hScoreY);
	lblHighScore.setFillColor(sf::Color::Yellow);
	sf::Text highScore(std::to_string(m_highScore), Assets::getInstance().getFont("Arial"), 40);
	highScore.setPosition(hScoreX, hScoreY + 50.f);
	highScore.setFillColor(sf::Color::Yellow);

	// Draw Time Bar

	std::vector<sf::RectangleShape> timeBars(m_timeCount);
	for (int i = 0; i < m_timeCount; ++i) {
		if (i < 9)
			timeBars[i].setFillColor(sf::Color::Red);
		else
			timeBars[i].setFillColor(sf::Color::Yellow);

		timeBars[i].setSize(sf::Vector2f(15, 48));
		timeBars[i].setOutlineColor(sf::Color::Black);
		timeBars[i].setOutlineThickness(2);
		timeBars[i].setPosition(timerX + i * 20, timerY);
	}

	// Draw Life
	auto& lives = m_entityManager.getEntities("life");
	for (auto& e : lives)
	{
		e->destroy();
	}
	for (int i = 0; i < m_lifeCount; i++)
	{
		auto life = m_entityManager.addEntity("life");
		life->addComponent<CAnimation>(Assets::getInstance().getAnimation("life"), true);
		life->addComponent<CTransform>(Vec2(lifeX + i * 70, lifeY), Vec2(0, 0), Vec2(1.5f, 1.5f));
	}

	auto& axeS = m_entityManager.getEntities("axeStatus");
	for (auto& e : axeS)
	{
		e->destroy();
	}
	std::string equipped = (m_player->getComponent<CInput>().canShoot) ? "axeGet" : "axeBlank";
	auto axeStatus = m_entityManager.addEntity("axeStatus");
	axeStatus->addComponent<CAnimation>(Assets::getInstance().getAnimation(equipped), true);
	axeStatus->addComponent<CTransform>(Vec2(lifeX + 15, lifeY + 70), Vec2(0, 0), Vec2(1.5f, 1.5f));

	m_game->window().draw(lblScore);
	m_game->window().draw(score);
	m_game->window().draw(lblHighScore);
	m_game->window().draw(highScore);
	for (int i = 0; i < m_timeCount; ++i) {
		if (i < m_remainingTime)
			m_game->window().draw(timeBars[i]);
		else
		{
			auto color = sf::Color::Black;
			color.a = 150;
			timeBars[i].setFillColor(color);
			m_game->window().draw(timeBars[i]);
		}
	}


}
