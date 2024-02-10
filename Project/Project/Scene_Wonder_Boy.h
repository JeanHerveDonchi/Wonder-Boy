#ifndef BREAKOUT_SCENE_BREAKOUT_H
#define BREAKOUT_SCENE_BREAKOUT_H

#include <SFML/System/Time.hpp>
#include <SFML/Graphics.hpp>
#include "EntityManager.h"
#include "Entity.h"
#include "Scene.h"
#include "GameEngine.h"


class Scene_Wonder_Boy : public Scene 
{
    struct PlayerConfig
    {
        float X{ 0.f }, Y{ 0.f }, CW{ 0.f }, CH{ 0.f };
        float SPEED{ 0.f }, MAXSPEED{ 0.f }, JUMP{ 0.f }, GRAVITY{ 0.f };
        std::string WEAPON;
    };
private:
    sPtrEntt        m_player{nullptr};
    std::string	    m_levelPath;
    PlayerConfig	m_playerConfig;

    sf::View        m_worldView;
    sf::FloatRect   m_worldBounds;
    


    //systems
    //void            sMovement(sf::Time dt);
    //void            sCollisions();
    //void            sAnimation(sf::Time dt);

    // Enemy Systems


    void	        onEnd() override;


    // helper functions
    void            spawnPlayer(sf::Vector2f pos);

    void            init();
    void            loadLevel(const std::string &path);

    sf::FloatRect   getViewBounds();

public:

    Scene_Wonder_Boy(GameEngine *gameEngine, const std::string &levelPath);

    void		  update(sf::Time dt) override;
    void		  sDoAction(const Command& command) override;
    void		  sRender() override;


};

static int             m_highScore{ 0 };

#endif //BREAKOUT_SCENE_BREAKOUT_H
