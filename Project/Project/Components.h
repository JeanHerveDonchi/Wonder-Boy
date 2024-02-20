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
    std::vector<Vec2> halfsizes{};

    CBoundingBox() = default;
    CBoundingBox(const Vec2& s) : size(s), halfSize(0.5f * s)
    {}
};

struct CState : public Component {

    enum playerState {
        isGrounded      = 1,        // 1
        isFacingLeft    = 1 << 1,   // 2
        isRuuning	    = 1 << 2,   // 4
        onSkate         = 1 << 3,   // 8
    };
    unsigned int state{ 0 };

    CState() = default;
    CState(unsigned int s) : state(s) {}
    // using bitset to test, set and unset state
    bool test(unsigned int x) { return (state & x); }
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


#endif //BREAKOUT_COMPONENTS_H
