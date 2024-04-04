#ifndef BREAKOUT_SCENE_BREAKOUT_H
#define BREAKOUT_SCENE_BREAKOUT_H

#include <SFML/System/Time.hpp>
#include <SFML/Graphics.hpp>
#include "EntityManager.h"
#include "Entity.h"
#include "Scene.h"
#include "GameEngine.h"
#include "Vec2.h"

const float GRID_SIZE = 96.f;

class Scene_Wonder_Boy : public Scene 
{
    struct PlayerConfig
    {
        float X{ 0.f }, Y{ 0.f }, CW{ 0.f }, CH{ 0.f };
        float SPEED{ 0.f }, MAXSPEED{ 0.f }, JUMP{ 0.f }, GRAVITY{ 0.f };
        std::string WEAPON;
    };
    struct WeaponConfig
    {
        float W{ 0.f }, H{ 0.f }, SPEED{ 0.f }, YVEL{ 0.f };
    };
private:
    sPtrEntt        m_player{nullptr};
    std::string	    m_levelPath;
    PlayerConfig	m_playerConfig;
    WeaponConfig	m_weaponConfig;
    sf::View        m_worldView;
    sf::View		m_hudView;
    sf::FloatRect   m_worldBounds;
    bool 		    m_drawTextures{ true };
    bool 		    m_drawColliders{ false };
    bool 		    m_drawGrid{ false };
    const Vec2      m_gridSize{ GRID_SIZE, GRID_SIZE };
    sf::Text        m_gridText;

    int             m_lifeCount{ 3 };
    int 		    m_score{ 0 };
    const int             m_timeCount{ 32 };
    int             m_remainingTime{ 0 };
    int             m_earnedTime{ 0 };
    sf::Clock       m_clock;

    Vec2			m_playerSpawnPos{ 2.f, 7.f };


    //systems
    void            sMovement(sf::Time dt);
    void            sCollisions();
    void            sAnimation(sf::Time dt);
    void	        onEnd() override;
    void            init(const std::string& levelPath);
    void            loadLevel(const std::string& path);
    void			registerActions();
    void            drawHUD();

    // helper functions
    void            spawnPlayer(Vec2 spawnPos = Vec2(2, 7));
    sf::FloatRect   getViewBounds();
    void            playerPostionAdjustment();

public:

    Scene_Wonder_Boy(GameEngine *gameEngine, const std::string &levelPath);

    void		  update(sf::Time dt) override;
    void		  sDoAction(const Command& command) override;
    void		  sRender() override;
    void          checkPlayerState();
    void          spawnBullet(std::shared_ptr<Entity> e);
    Vec2          gridToMidPixel(float gridX, float gridY, sPtrEntt entity);
    void          sLifeSpan();
    void          sEnemyAI();

};

static int             m_highScore{ 0 };

#endif //BREAKOUT_SCENE_BREAKOUT_H
