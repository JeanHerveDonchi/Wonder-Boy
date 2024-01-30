
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

const sf::Vector2f SCORE_POS = sf::Vector2f{ 380.f, 30.f };
const sf::Vector2f HIGHSCORE_POS = sf::Vector2f{ 230.f, 30.f };
const sf::Vector2f TIMER_POS = sf::Vector2f{ 140.f, 30.f };

const sf::Time TIME_PER_INPUT = sf::seconds(0.3f);

namespace
{
	std::random_device rd;
	std::mt19937 rng(rd());
}


Scene_Wonder_Boy::Scene_Wonder_Boy(GameEngine* gameEngine, const std::string& levelPath)
	: Scene(gameEngine)
	, m_worldView(gameEngine->window().getDefaultView())
{
	loadLevel(levelPath);
	registerActions();

	init();

	MusicPlayer::getInstance().play("gameTheme");
	MusicPlayer::getInstance().setVolume(50);
}


void Scene_Wonder_Boy::init()
{
	auto pos = m_worldView.getSize();

	// spawn frog in middle of first row
	pos.x = pos.x / 2.f;
	pos.y -= 20.f;

	
	spawnLives();

	spawnPlayer(pos);

	m_clock.restart();

	m_highScore = 0;
}

void Scene_Wonder_Boy::sMovement(sf::Time dt)
{
	
	playerMovement();

	// move all objects
	for (auto e : m_entityManager.getEntities())
	{

		if (e->hasComponent<CTransform>())
		{
			auto& tfm = e->getComponent<CTransform>();

			tfm.pos += tfm.vel * dt.asSeconds();
			tfm.angle += tfm.angVel * dt.asSeconds();
		}
	}
}


void Scene_Wonder_Boy::registerActions()
{
	registerAction(sf::Keyboard::P, "PAUSE");
	registerAction(sf::Keyboard::Escape, "BACK");
	registerAction(sf::Keyboard::Q, "QUIT");
	registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");

	registerAction(sf::Keyboard::A, "LEFT");
	registerAction(sf::Keyboard::Left, "LEFT");
	registerAction(sf::Keyboard::D, "RIGHT");
	registerAction(sf::Keyboard::Right, "RIGHT");
	registerAction(sf::Keyboard::W, "UP");
	registerAction(sf::Keyboard::Up, "UP");
	registerAction(sf::Keyboard::S, "DOWN");
	registerAction(sf::Keyboard::Down, "DOWN");
}


void Scene_Wonder_Boy::onEnd()
{
	m_game->changeScene("MENU", nullptr, false);
}

void Scene_Wonder_Boy::playerMovement()
{
	// no movement if player is dead
	if (m_player->hasComponent<CState>() && m_player->getComponent<CState>().state == "dead")
		return;

	sManageScore();


	auto& dir = m_player->getComponent<CInput>().dir;
	auto& pos = m_player->getComponent<CTransform>().pos;

	if (dir & CInput::UP)
	{
		m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("up"));
		pos.y -= 40.f;
	}
	if (dir & CInput::DOWN)
	{
		m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("down"));
		pos.y += 40.f;
	}

	if (dir & CInput::LEFT)
	{
		m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("left"));
		pos.x -= 40.f;
	}

	if (dir & CInput::RIGHT)
	{
		m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("right"));
		pos.x += 40.f;
	}

	if (dir != 0)
	{
		SoundPlayer::getInstance().play("hop", m_player->getComponent<CTransform>().pos);
		dir = 0;
	}
}


void Scene_Wonder_Boy::sRender()
{
	m_game->window().setView(m_worldView);

	// draw bkg first
	for (auto e : m_entityManager.getEntities("bkg"))
	{
		if (e->getComponent<CSprite>().has)
		{
			auto& sprite = e->getComponent<CSprite>().sprite;
			m_game->window().draw(sprite);
		}
	}

	for (auto& e : m_entityManager.getEntities())
	{
		if (!e->hasComponent<CAnimation>())
			continue;

		// Draw Sprite
		auto& anim = e->getComponent<CAnimation>().animation;
		auto& tfm = e->getComponent<CTransform>();
		anim.getSprite().setPosition(tfm.pos);
		anim.getSprite().setRotation(tfm.angle);
		m_game->window().draw(anim.getSprite());

		if (m_drawAABB)
		{
			if (e->hasComponent<CBoundingBox>())
			{
				auto box = e->getComponent<CBoundingBox>();
				sf::RectangleShape rect;
				rect.setSize(sf::Vector2f{ box.size.x, box.size.y });
				centerOrigin(rect);
				rect.setPosition(e->getComponent<CTransform>().pos);
				rect.setFillColor(sf::Color(0, 0, 0, 0));
				rect.setOutlineColor(sf::Color{ 0, 255, 0 });
				rect.setOutlineThickness(2.f);
				m_game->window().draw(rect);
			}
		}

		// Draw Score
		sf::Text lblScore("Score", Assets::getInstance().getFont("Arial"), 15);
		lblScore.setPosition(SCORE_POS - sf::Vector2f{ 0.f, 20.f });
		sf::Text score(std::to_string(m_score), Assets::getInstance().getFont("Arial"), 15);
		score.setPosition(SCORE_POS);
		score.setFillColor(sf::Color::Magenta);

		// Draw High Score
		sf::Text lblHighScore("High Score", Assets::getInstance().getFont("Arial"), 12);
		lblHighScore.setPosition(HIGHSCORE_POS - sf::Vector2f{ 0.f, 20.f });
		lblHighScore.setFillColor(sf::Color::Yellow);
		sf::Text highScore(std::to_string(m_highScore), Assets::getInstance().getFont("Arial"), 15);
		highScore.setPosition(HIGHSCORE_POS);
		highScore.setFillColor(sf::Color::Yellow);

		// Draw Timer
		sf::Text lblTime("Time", Assets::getInstance().getFont("Arial"), 12);
		lblTime.setPosition(TIMER_POS - sf::Vector2f{ 0.f, 20.f });
		sf::Text time(std::to_string(m_timeCount), Assets::getInstance().getFont("Arial"), 15);
		time.setPosition(TIMER_POS);
		if (m_timeCount < 10)
		{
			time.setFillColor(sf::Color::Red);
			time.Bold;
			time.setCharacterSize(20);
		}
		else
		{
			time.setFillColor(sf::Color::Blue);
		}

		m_game->window().draw(lblScore);
		m_game->window().draw(score);
		m_game->window().draw(lblHighScore);
		m_game->window().draw(highScore);
		m_game->window().draw(lblTime);
		m_game->window().draw(time);


		// Render paused message
		if (m_lifeCount == 0 && m_isPaused)
		{
			sf::Text pausedMsg("Do you want to quit? \n Q (quit) \n Press a space key (continue)", Assets::getInstance().getFont("Arial"), 30);
			centerOrigin(pausedMsg);
			pausedMsg.setPosition(m_worldView.getCenter());
			pausedMsg.setFillColor(sf::Color::Yellow);
			m_game->window().draw(pausedMsg);
		}
		else if (m_isPaused)
		{
			sf::Text pausedMsg("Paused", Assets::getInstance().getFont("Arial"), 30);
			centerOrigin(pausedMsg);
			pausedMsg.setPosition(m_worldView.getCenter());
			pausedMsg.setFillColor(sf::Color::Yellow);
			m_game->window().draw(pausedMsg);
		}
	}
}


void Scene_Wonder_Boy::update(sf::Time dt)
{
	sUpdate(dt);
}

void Scene_Wonder_Boy::sDoAction(const Command& action)
{
	// On Key Press
	if (action.type() == "START")
	{
		if (action.name() == "PAUSE") { setPaused(!m_isPaused); }
		else if (action.name() == "QUIT") { m_game->quitLevel(); }
		else if (action.name() == "BACK") { m_game->backLevel(); }

		else if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
		else if (action.name() == "TOGGLE_COLLISION") { m_drawAABB = !m_drawAABB; }
		else if (action.name() == "TOGGLE_GRID") { m_drawGrid = !m_drawGrid; }

		// Player control
		// Not using them, overwriting them => to use only 1 direction at a time, 
		// if multiple keys are pressed, only the last one is used (during 16ms, the time between frames)
		if (action.name() == "LEFT") { m_player->getComponent<CInput>().dir = CInput::LEFT; }
		else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().dir = CInput::RIGHT; }
		else if (action.name() == "UP") { m_player->getComponent<CInput>().dir = CInput::UP; }
		else if (action.name() == "DOWN") { m_player->getComponent<CInput>().dir = CInput::DOWN; }
	}
	// on Key Release
	// the frog can only go in one direction at a time, no angles
	// use a bitset and exclusive setting.
	else if (action.type() == "END" && (action.name() == "LEFT" || action.name() == "RIGHT" || action.name() == "UP" ||
		action.name() == "DOWN"))
	{
		m_player->getComponent<CInput>().dir = 0;
	}
}


void Scene_Wonder_Boy::spawnPlayer(sf::Vector2f pos)
{
	m_player = m_entityManager.addEntity("player");
	m_player->addComponent<CTransform>(pos);
	m_player->addComponent<CBoundingBox>(sf::Vector2f(15.f, 15.f));
	m_player->addComponent<CInput>();
	m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("up"));
	m_player->addComponent<CState>("alive");
}


void Scene_Wonder_Boy::spawnLives()
{
	for (int i{ 0 }; i < m_lifeCount; ++i)
	{
		auto life = m_entityManager.addEntity("lives");
		life->addComponent<CTransform>(sf::Vector2f{ 60.f - 20.f * i, 20.f });
		life->addComponent<CAnimation>(Assets::getInstance().getAnimation("lives"));
	}
}



sf::FloatRect Scene_Wonder_Boy::getViewBounds()
{
	return sf::FloatRect();
}

void Scene_Wonder_Boy::sCollisions()
{


	auto entities = m_entityManager.getEntities();
	auto carentities = m_entityManager.getEntities("car");
	auto turtleentities = m_entityManager.getEntities("turtle");
	auto logentities = m_entityManager.getEntities("log");
	auto lillyPadentities = m_entityManager.getEntities("lillyPad");

	auto playerBox = m_player->getComponent<CBoundingBox>();
	auto& playerTfm = m_player->getComponent<CTransform>();

	auto state = m_player->getComponent<CState>().state;
	const int LINE = 320;
	const int LILLYPAD_LINE = 110;

	if (state != "dead")
	{

		if (playerTfm.pos.y > LINE)
		{
			for (auto e : carentities)
			{
				auto carBox = e->getComponent<CBoundingBox>();
				auto carTfm = e->getComponent<CTransform>();
				auto overlap = Physics::getOverlap(m_player, e);

				if (overlap.x > 0 && overlap.y > 0)
				{
					m_player->getComponent<CState>().state = "dead";
					m_player->removeComponent<CBoundingBox>();
					m_player->getComponent<CAnimation>().animation = Assets::getInstance().getAnimation("die");
					SoundPlayer::getInstance().play("death", m_player->getComponent<CTransform>().pos);
					break;
				}
			}
		}
		else if (playerTfm.pos.y < LINE && playerTfm.pos.y > LILLYPAD_LINE)
		{
			if (playerTfm.pos.x == 20 || playerTfm.pos.x == 460)
			{
				m_player->getComponent<CState>().state = "dead";
				m_player->removeComponent<CBoundingBox>();
				m_player->getComponent<CAnimation>().animation = Assets::getInstance().getAnimation("die");
				SoundPlayer::getInstance().play("death", m_player->getComponent<CTransform>().pos);
				return;
			}

			m_player->getComponent<CState>().state = "dead";
			for (auto e : turtleentities)
			{
				auto turtleBox = e->getComponent<CBoundingBox>();
				auto turtleTfm = e->getComponent<CTransform>();
				auto overlap = Physics::getOverlap(m_player, e);

				if (overlap.x > 0 && overlap.y > 0)
				{
					playerTfm.vel.x = turtleTfm.vel.x;
					m_player->getComponent<CState>().state = "alive";
					break;
				}
			}
			for (auto e : logentities)
			{
				auto logBox = e->getComponent<CBoundingBox>();
				auto logTfm = e->getComponent<CTransform>();
				auto overlap = Physics::getOverlap(m_player, e);

				if (overlap.x > 0 && overlap.y > 0)
				{
					playerTfm.vel.x = logTfm.vel.x;
					m_player->getComponent<CState>().state = "alive";
				}
			}
			if (m_player->getComponent<CState>().state == "dead")
			{
				m_player->removeComponent<CBoundingBox>();
				m_player->getComponent<CAnimation>().animation = Assets::getInstance().getAnimation("die");
				SoundPlayer::getInstance().play("death", m_player->getComponent<CTransform>().pos);
			}
		}
		else if (playerTfm.pos.y < LILLYPAD_LINE)
		{
			m_player->getComponent<CState>().state = "dead";
			for (auto e : lillyPadentities)
			{
				auto lillyPadBox = e->getComponent<CBoundingBox>();
				auto lillyPadTfm = e->getComponent<CTransform>();
				auto overlap = Physics::getOverlap(m_player, e);

				if (overlap.x > 0 && overlap.y > 0)
				{
					m_player->getComponent<CState>().state = "destinationReached";
					e->removeComponent<CBoundingBox>();
					auto reachedFrog = m_entityManager.addEntity("reachedFrog");
					reachedFrog->addComponent<CTransform>(lillyPadTfm.pos);
					reachedFrog->addComponent<CAnimation>(Assets::getInstance().getAnimation("frogIcon"));

					break;
				}
			}
		}

		if (m_player->getComponent<CState>().state == "dead")
		{
			m_player->removeComponent<CBoundingBox>();
			m_player->getComponent<CAnimation>().animation = Assets::getInstance().getAnimation("die");
			SoundPlayer::getInstance().play("death", m_player->getComponent<CTransform>().pos);
		}
	}

}


void Scene_Wonder_Boy::checkRestart()
{
	if (m_lifeCount == 0 && m_isPaused)
	{

		/*MusicPlayer::getInstance().stop();

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		{
			MusicPlayer::getInstance().setVolume(50);
			MusicPlayer::getInstance().play("gameTheme"); 
			Scene::m_isPaused = false;
			m_lifeCount = 3;
			spawnLives();
			m_score = 0;
			auto reachedFrogs = m_entityManager.getEntities("reachedFrog");
			for (auto e : reachedFrogs)
			{
				e->destroy();
			}
			auto lilypads = m_entityManager.getEntities("lillyPad");
			for (auto e : lilypads)
			{
				if (!e->hasComponent<CBoundingBox>())
					e->addComponent<CBoundingBox>(sf::Vector2f{ LILLYPAD_BB });	
			}
			
			return;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
		{
			m_game->changeScene("MENU", nullptr, false);
		}*/
	}
}

void Scene_Wonder_Boy::sUpdate(sf::Time dt)
{

	checkRestart();

	SoundPlayer::getInstance().removeStoppedSounds();
	m_entityManager.update();

	

	if (m_isPaused)
		return;
	
	sAnimation(dt);
	sMovement(dt);
	

	adjustPlayerPosition();
	sCollisions();
	sManageTimer();

	checkPlayerState();
	
}


void Scene_Wonder_Boy::sAnimation(sf::Time dt)
{
	auto list = m_entityManager.getEntities();
	for (auto e : m_entityManager.getEntities())
	{
		// update all animations
		if (e->hasComponent<CAnimation>())
		{
			auto& anim = e->getComponent<CAnimation>();
			anim.animation.update(dt);
			// do nothing if animation has ended
		}
	}
}

#pragma region Enemy Behaviour
// Enemy Behaviour

void Scene_Wonder_Boy::sManageSnail()
{
}

void Scene_Wonder_Boy::sManageHornet()
{
}

void Scene_Wonder_Boy::sManageCobra()
{
}

void Scene_Wonder_Boy::sManageFrog()
{
}

void Scene_Wonder_Boy::sManageOctopus()
{
}

void Scene_Wonder_Boy::sManageBat()
{
}

void Scene_Wonder_Boy::sManageSkeleton()
{
}

void Scene_Wonder_Boy::sManageNative()
{
}

void Scene_Wonder_Boy::sManageSpider()
{
}

void Scene_Wonder_Boy::sManageGoblin()
{
}

void Scene_Wonder_Boy::sManageBluefish()
{
}

void Scene_Wonder_Boy::sManageRock()
{
}

void Scene_Wonder_Boy::sManageBonfire()
{
}

void Scene_Wonder_Boy::sManageIcicle()
{
}

void Scene_Wonder_Boy::sManageBoulder()
{
}

#pragma endregion


void Scene_Wonder_Boy::sManageTimer()
{
	if (m_clock.getElapsedTime().asSeconds() > 1)
	{
		m_clock.restart();
		m_timeCount--;
	}

	if (m_timeCount <= 0)
	{
		m_player->getComponent<CState>().state = "dead";
		m_player->removeComponent<CBoundingBox>();
		m_player->getComponent<CAnimation>().animation = Assets::getInstance().getAnimation("die");
		SoundPlayer::getInstance().play("death", m_player->getComponent<CTransform>().pos);

		m_timeCount = 40;
	}


}

void Scene_Wonder_Boy::sManageScore()
{
	if (m_score > m_highScore)
	{
		m_highScore = m_score;
	}

	auto playerPosY = m_player->getComponent<CTransform>().pos.y;

	if (playerPosY < m_reachDistance)
	{
		m_score += 10;
		m_reachDistance = playerPosY;
	}
	
}



void Scene_Wonder_Boy::adjustPlayerPosition()
{
	auto center = m_worldView.getCenter();
	sf::Vector2f viewHalfSize = m_worldView.getSize() / 2.f;


	auto left = center.x - viewHalfSize.x;
	auto right = center.x + viewHalfSize.x;
	auto top = center.y - viewHalfSize.y;
	auto bot = center.y + viewHalfSize.y;

	auto& player_pos = m_player->getComponent<CTransform>().pos;
	auto halfSize = sf::Vector2f{ 20, 20 };
	// keep player in bounds
	player_pos.x = std::max(player_pos.x, left + halfSize.x);
	player_pos.x = std::min(player_pos.x, right - halfSize.x);
	player_pos.y = std::max(player_pos.y, top + halfSize.y);
	player_pos.y = std::min(player_pos.y, bot - halfSize.y);
}

void Scene_Wonder_Boy::checkPlayerState()
{
	if (m_player->getComponent<CState>().state == "dead")
	{
		m_reachDistance = 560.0f;

		auto& anim = m_player->getComponent<CAnimation>().animation;
		if (anim.hasEnded())
		{
			// respawn player
			m_player->getComponent<CAnimation>().animation = Assets::getInstance().getAnimation("up");
			m_player->getComponent<CTransform>().pos = sf::Vector2f{ 240.f, 580.f };
			m_player->getComponent<CTransform>().vel = sf::Vector2f{ 0.f, 0.f };
			m_player->getComponent<CState>().state = "alive";
			m_player->addComponent<CBoundingBox>(sf::Vector2f{ 15.f, 15.f });

			
			// with deducting a life
			m_lifeCount--;
			auto lives = m_entityManager.getEntities("lives");
			lives[m_lifeCount]->destroy();

			m_clock.restart();
			m_timeCount = 40;

			if (m_lifeCount == 0)
			{
				Scene::m_isPaused = true;						
			}
		}
	}
	if (m_player->getComponent<CState>().state == "destinationReached")
	{
		m_reachDistance = 560.0f;
		// respawn player withour deducting a life
		m_player->getComponent<CAnimation>().animation = Assets::getInstance().getAnimation("up");
		m_player->getComponent<CTransform>().pos = sf::Vector2f{ 240.f, 580.f };
		m_player->getComponent<CTransform>().vel = sf::Vector2f{ 0.f, 0.f };
		m_player->getComponent<CState>().state = "alive";
		m_player->addComponent<CBoundingBox>(sf::Vector2f{ 15.f, 15.f });


		m_score += 50;

		m_clock.restart();
		m_timeCount = 40;
	}
}

void Scene_Wonder_Boy::loadLevel(const std::string& path)
{
	std::ifstream config(path);
	if (config.fail())
	{
		std::cerr << "Open file " << path << " failed\n";
		config.close();
		exit(1);
	}

	std::string token{ "" };
	config >> token;
	while (!config.eof())
	{
		if (token == "Bkg")
		{
			std::string name;
			sf::Vector2f pos;
			config >> name >> pos.x >> pos.y;
			auto e = m_entityManager.addEntity("bkg");

			// for background, no textureRect its just the whole texture
			// and no center origin, position by top left corner
			// stationary so no CTransfrom required.
			auto& sprite = e->addComponent<CSprite>(Assets::getInstance().getTexture(name)).sprite;
			sprite.setOrigin(0.f, 0.f);
			sprite.setPosition(pos);
		}
		else if (token[0] == '#')
		{
			std::cout << token;
		}

		config >> token;
	}

	config.close();
}
