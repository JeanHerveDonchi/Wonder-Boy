//
// Created by David Burchill on 2023-09-27.
//

#ifndef BREAKOUT_SCENE_BREAKOUT_H
#define BREAKOUT_SCENE_BREAKOUT_H

#include <SFML/System/Time.hpp>
#include <SFML/Graphics.hpp>
#include "EntityManager.h"
#include "Entity.h"
#include "Scene.h"
#include "GameEngine.h"


class Scene_Wonder_Boy : public Scene {
private:
    sPtrEntt        m_player{nullptr};
    sf::View        m_worldView;
    sf::FloatRect   m_worldBounds;

    bool			m_drawTextures{true};
    bool			m_drawAABB{false};
    bool			m_drawGrid{false};

    //
    int             m_lifeCount{3};
    int 		    m_score{0};
    int             m_timeCount{ 40 };
    sf::Clock       m_clock;
    float           m_reachDistance{ 560.0f }; 


    //systems
    void            sMovement(sf::Time dt);
    void            sCollisions();
    void            sUpdate(sf::Time dt);
    void            sAnimation(sf::Time dt);

    // Enemy Systems
    void            sManageSnail();
    void            sManageHornet();
    void            sManageCobra();
    void            sManageFrog();
    void            sManageOctopus();
    void            sManageBat();
    void            sManageSkeleton();
    void            sManageNative();
    void            sManageSpider();
    void            sManageGoblin();
    void            sManageBluefish();
    void            sManageRock();
    void            sManageBonfire();
    void            sManageIcicle();
    void            sManageBoulder();


    void            sManageTimer();
    void            sManageScore();

    void	        onEnd() override;


    // helper functions
    void            playerMovement();
    void            adjustPlayerPosition();
    void            adjustCameraPosition();
    void            checkPlayerState();
    void	        registerActions();
    void            spawnPlayer(sf::Vector2f pos);

    void            spawnSnail();
    void            spawnHornet();
	void            spawnCobra();
    void            spawnFrog();
    void            spawnOctopus();
    void            spawnBat();
    void            spawnSkeleton();
    void            spawnNative();
    void            spawnSpider();
    void            spawnGoblin();
    void            spawnBluefish();
    void            spawnRock();
    void            spawnBonfire();
    void            spawnIcicle();
    void            spawnBoulder();

    
    void            spawnLives();

    void            checkRestart();

    void            getItems();

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
