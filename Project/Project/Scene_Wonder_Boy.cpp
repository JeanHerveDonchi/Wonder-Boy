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

	init();

}


void Scene_Wonder_Boy::init()
{

	auto pos = m_worldView.getSize();
	
	spawnPlayer(pos / 2.f);


	MusicPlayer::getInstance().play("level01");
	MusicPlayer::getInstance().setVolume(50);
}



void Scene_Wonder_Boy::onEnd()
{;
	m_game->changeScene("MENU", nullptr, false);
}


void Scene_Wonder_Boy::sRender()
{
	m_game->window().setView(m_worldView);

	static const sf::Color backgroundColor(0x23, 0xc7, 0xc9);
	m_game->window().clear(backgroundColor);

	auto& entities = m_entityManager.getEntities();
	for (auto& e : entities)
	{
		if (e->hasComponent<CSprite>())
		{
			auto& eCSprite = e->getComponent<CSprite>();
			m_game->window().draw(eCSprite.sprite);
		}
	}
}


void Scene_Wonder_Boy::update(sf::Time dt)
{


	SoundPlayer::getInstance().removeStoppedSounds();
	m_entityManager.update();



	if (m_isPaused)
		return;


}

void Scene_Wonder_Boy::sDoAction(const Command& command)
{
}

void Scene_Wonder_Boy::spawnPlayer(sf::Vector2f pos)
{
	m_player->addComponent<CTransform>(pos);

}


sf::FloatRect Scene_Wonder_Boy::getViewBounds()
{
	return sf::FloatRect();
}



void Scene_Wonder_Boy::loadLevel(const std::string& path)
{
	std::ifstream levelConfig(path);
	if (levelConfig.fail())
	{
		std::cerr << "Open file " << path << " failed\n";
		levelConfig.close();
		exit(1);
	}

	std::string token{ "" };
	levelConfig >> token;

	while (!levelConfig.eof())
	{
		

		if (token == "Player") {

			std::string playerTexturePath;
			levelConfig >> playerTexturePath;
			sf::Texture playerTexture;
			if (!playerTexture.loadFromFile(playerTexturePath)) {
				std::cerr << "Error loading testPlayer.png\n";
				exit(1);
			}

			m_player = m_entityManager.addEntity("Player");

			auto& pCSprite = m_player->addComponent<CSprite>(playerTexture);
		}

		levelConfig >> token;
	}

	levelConfig.close();
}
