#include "Scene_Menu.h"
#include "Scene_Wonder_Boy.h"
#include <memory>

void Scene_Menu::onEnd()
{
	m_game->window().close();
}

Scene_Menu::Scene_Menu(GameEngine* gameEngine)
	: Scene(gameEngine)
{
	init();
}



void Scene_Menu:: init()
{
    registerAction(sf::Keyboard::W,			"UP");
    registerAction(sf::Keyboard::Up,		"UP");
    registerAction(sf::Keyboard::S,			"DOWN");
    registerAction(sf::Keyboard::Down,	 	"DOWN");
	registerAction(sf::Keyboard::D,			"PLAY");
	registerAction(sf::Keyboard::Escape,	"QUIT");

	m_title = "Wonder Boy Project";
	m_menuStrings.push_back("Start");
	m_menuStrings.push_back("High Scores");
	m_menuStrings.push_back("quit");

	m_menuText.setFont(Assets::getInstance().getFont("main"));

	const size_t CHAR_SIZE{ 64 };
	m_menuText.setCharacterSize(CHAR_SIZE);

}

void Scene_Menu::update(sf::Time dt)
{
	m_entityManager.update();
}


void Scene_Menu::sRender()
{
	 
	sf::View view = m_game->window().getView();
	view.setCenter(m_game->window().getSize().x / 2.f, m_game->window().getSize().y / 2.f);
	m_game->window().setView(view);

	static const sf::Color selectedColor(255, 255, 255);
	static const sf::Color normalColor(0, 0, 0);

	static const sf::Color backgroundColor(100, 100, 255);

	sf::Text footer("UP: W    DOWN: S   PLAY:D    QUIT: ESC", 
		Assets::getInstance().getFont("main"), 20);
	footer.setFillColor(normalColor);
	footer.setPosition(60, 800);

	m_game->window().clear(backgroundColor);

	// draw bkg first
	sf::Texture bkgTexture;
	if (!bkgTexture.loadFromFile("../Assets/Textures/Title_Screen.png")) {
		std::cerr << "Error loading title_Screen.png\n";

		exit(1);
	}

	sf::Sprite bkgSprite(bkgTexture);
	bkgSprite.setOrigin(bkgSprite.getLocalBounds().width / 2, bkgSprite.getLocalBounds().height / 2);
	bkgSprite.setPosition(m_game->window().getSize().x / 1.5, m_game->window().getSize().y / 2);
	bkgSprite.setScale(5, 4);
	m_game->window().draw(bkgSprite);

	m_menuText.setFillColor(normalColor);
	m_menuText.setString(m_title);
	m_menuText.setPosition(30, 70);
	m_game->window().draw(m_menuText);

	for (size_t i{ 0 }; i < m_menuStrings.size(); ++i)
	{
		m_menuText.setFillColor((i == m_menuIndex ? selectedColor : normalColor));
		m_menuText.setPosition(60, 180 + (i+1) * 96);
		m_menuText.setString(m_menuStrings.at(i));
		m_game->window().draw(m_menuText);
	} 

	m_game->window().draw(footer);

}


void Scene_Menu::sDoAction(const Command& action)
{
	if (action.type() == "START")
	{
		if (action.name() == "UP")
		{
			m_menuIndex = (m_menuIndex + m_menuStrings.size() - 1) % m_menuStrings.size();
		} 
		else if (action.name() == "DOWN")
		{
			m_menuIndex = (m_menuIndex + 1) % m_menuStrings.size();
		}
		else if (action.name() == "PLAY")
		{
			m_game->changeScene("PLAY", std::make_shared<Scene_Wonder_Boy>(m_game, m_levelPaths[m_menuIndex]));
		}
		else if (action.name() == "QUIT")
		{
			onEnd();
		}
	}

}
