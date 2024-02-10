#ifndef SFMLCLASS_ANIMATION_H
#define SFMLCLASS_ANIMATION_H

#include "Common.h"

class Animation
{
private:
	sf::Sprite m_sprite;
	size_t m_frameCount{ 1 };
	size_t m_currentFrame{ 0 };
	size_t m_speed{ 0 };
	Vec2   m_size{ 1,1 };
	std::string m_name{ "NONE" };
	bool   m_hasEnded{ false };

public:
	Animation();
	Animation(const std::string& name, const sf::Texture& t); // single frame
	Animation(const std::string& name, const sf::Texture& t, size_t frameCount, size_t speed); // multiple frames

	void				update(bool repeat = true);
	bool				hasEnded() const;
	sf::Sprite&			getSprite();
	const std::string&	getName() const;
	const Vec2			getSize() const;
};


#endif //SFMLCLASS_ANIMATION_H
