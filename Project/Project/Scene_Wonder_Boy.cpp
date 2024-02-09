
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

	MusicPlayer::getInstance().play("level01");
	MusicPlayer::getInstance().setVolume(50);
}


void Scene_Wonder_Boy::init()
{

	auto pos = m_worldView.getSize();
	
	spawnPlayer(pos / 2.f);
}



void Scene_Wonder_Boy::onEnd()
{
	m_game->changeScene("MENU", nullptr, false);
}


void Scene_Wonder_Boy::sRender()
{
	m_game->window().setView(m_worldView);

	static const sf::Color backgroundColor(100, 100, 255);
	m_game->window().clear(backgroundColor);



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
	m_player->getComponent<CTransform>().pos = pos;

}


sf::FloatRect Scene_Wonder_Boy::getViewBounds()
{
	return sf::FloatRect();
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

	static const sf::Color backgroundColor(100, 100, 255);
	while (!config.eof())
	{
		if (token == "Bkg")
		{

		}
		else if (token == "Player")
		{
			std::string name;
			sf::Vector2f pos;
			config >> name;
			auto e = m_entityManager.addEntity("player");
			m_player = e;

			auto& sprite = e->addComponent<CSprite>(Assets::getInstance().getTexture(name)).sprite;
			sprite.setPosition(pos);
			sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
			sprite.setScale(2.f, 2.f);

		}
		else if (token[0] == '#')
		{
			std::cout << token;
		}

		config >> token;
	}

	config.close();
}
