
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

	MusicPlayer::getInstance().play("level01");
	MusicPlayer::getInstance().setVolume(50);
}


void Scene_Wonder_Boy::init()
{
	auto pos = m_worldView.getSize();
	

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
	registerAction(sf::Keyboard::W, "JUMP");
	registerAction(sf::Keyboard::Up, "JUMP");
	registerAction(sf::Keyboard::Space, "THROW_WEAPON");
}


void Scene_Wonder_Boy::onEnd()
{
	m_game->changeScene("MENU", nullptr, false);
}

void Scene_Wonder_Boy::playerMovement()
{

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
	
}


void Scene_Wonder_Boy::spawnPlayer(sf::Vector2f pos)
{
	
}


void Scene_Wonder_Boy::spawnLives()
{
	
}



sf::FloatRect Scene_Wonder_Boy::getViewBounds()
{
	return sf::FloatRect();
}

void Scene_Wonder_Boy::sCollisions()
{

}


void Scene_Wonder_Boy::checkRestart()
{
	
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
	

}

void Scene_Wonder_Boy::sManageScore()
{
	
}



void Scene_Wonder_Boy::adjustPlayerPosition()
{
	
}

void Scene_Wonder_Boy::checkPlayerState()
{
	
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

			sf::Vector2u windowSize = m_game->window().getSize();

			sprite.setTextureRect(sf::IntRect(0, 0, windowSize.x, windowSize.y));
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
