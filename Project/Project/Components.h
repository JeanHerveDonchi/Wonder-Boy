#ifndef BREAKOUT_COMPONENTS_H
#define BREAKOUT_COMPONENTS_H


#include <memory>
#include <SFML/Graphics.hpp>
#include "Utilities.h"
#include "Animation.h"
#include <bitset>


struct Component
{
    bool		has{ false };
    Component() = default;
};


struct CAnimation : public Component 
{
    Animation   animation;
    bool repeat{ false };

    CAnimation() = default;
    CAnimation(const Animation& a, bool r) : animation(a), repeat(r) {}

};

struct CSprite : public Component 
{
    sf::Sprite sprite;

    CSprite() = default;

    CSprite(const sf::Texture &t)
            : sprite(t) {
        centerOrigin(sprite);
    }

    CSprite(const sf::Texture &t, sf::IntRect r)
            : sprite(t, r) {
        centerOrigin(sprite);
    }
};


struct CTransform : public Component
{

    sf::Transformable  tfm;
    Vec2	pos			{ 0.f, 0.f };
    Vec2	prevPos		{ 0.f, 0.f };
    Vec2	vel			{ 0.f, 0.f };
    Vec2	scale		{ 1.f, 1.f };

    float           angVel{ 0 };
    float	        angle{ 0.f };

    CTransform() = default;
    CTransform(const Vec2& p) : pos(p)  {}
    CTransform(const Vec2& p, const Vec2& v)
            : pos(p), prevPos(p),  vel(v){}
    CTransform(const Vec2& p, const Vec2& v, const Vec2& sc)
        : pos(p), prevPos(p), vel(v), scale(sc) {}
    CTransform(const Vec2& p, const Vec2& v, const Vec2& sc, float a)
        : pos(p), prevPos(p), vel(v), scale(sc), angle(a) {}

};



struct CBoundingBox : public Component
{
    Vec2 size{0.f, 0.f};
    Vec2 halfSize{ 0.f, 0.f };

    CBoundingBox() = default;
    CBoundingBox(const Vec2& s) : size(s), halfSize(0.5f * s)
    {}
};

struct CBoundingBoxes : public Component
{
    std::vector<Vec2> sizes{};
	std::vector<Vec2> halfsizes{};

	CBoundingBoxes() = default;
	CBoundingBoxes(const std::vector<Vec2>& hs) : halfsizes(hs) {}

};

struct CState : public Component {

    enum playerState {
        isAlive         = 1,        // 1 set: alive,  unset: dead
        isFacingLeft    = 1 << 1,   // 2 set: facing left,  unset: facing right
        isRunning	    = 1 << 2,   // 4 set: running,  unset: not running, stand
        onSkate         = 1 << 3,   // 8 set: on skate,  unset: not on skate
        isGrounded      = 1 << 4,   // 16 set: grounded,  unset: not grounded
        isThrowing	    = 1 << 5,   // 32 set: throwing,  unset: not throwing
        isBurned        = 1 << 6,   // 64 set: burned,  unset: not burned
        isTripping      = 1 << 7,   // 128 set: tripping,  unset: not tripping
    };
    unsigned int state{ 0 }; // to set 0 to compare with each bit state

    CState() = default;
    CState(unsigned int s) : state(s) {}
    // using bitset to test, set and unset state
    bool test(unsigned int x) { return (state & x); }  // test if bit is set, & is bitwise AND, 
    void set(unsigned int x) { state |= x; }
    void unSet(unsigned int x) { state &= ~x; }

};


struct CInput : public Component
{
    bool jump{ false };
    bool left{ false };
    bool right{ false };
    bool shoot{ false };
    bool canShoot{ false };
    bool canJump{ true };

    CInput() = default;
};

struct CPhysics : public Component
{
    float		gravity{ 0.f };
    float		maxSpeed{ 0.f };
    float		speed{ 0.f };
    float		jump{ 0.f };

    CPhysics() = default;
    CPhysics(float g) : gravity(g) {}
    CPhysics(float g, float m, float s, float j)
			: gravity(g), maxSpeed(m), speed(s), jump(j) {}
};

struct CLifespan : public Component
{
	int    lifeTime;
	int    remainingTime;

	CLifespan() = default;
	CLifespan(int t) : lifeTime(t), remainingTime(t) {}
};

struct CAI : public Component
{
    bool detectPlayer{ false };
    float velX{ 0.f };
    bool moveVertically{ false };
    float velY{ 0.f };
    float maxY{ 0.f };
    float minY{ 0.f };

    CAI() = default;
    CAI(float vx)	: detectPlayer(false), velX(vx), moveVertically(false), maxY(0.f), minY(0.f) {}
    CAI(float vx, float vy)
        : detectPlayer(false), velX(vx), moveVertically(true), velY(vy), maxY(980.f), minY(979.f) {}
    CAI(float vx, bool mv, float vy, float max, float min)
		: detectPlayer(false), velX(vx), moveVertically(mv), velY(vy), maxY(max), minY(min) {}
};
#endif //BREAKOUT_COMPONENTS_H
