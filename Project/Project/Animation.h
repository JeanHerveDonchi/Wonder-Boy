#ifndef SFMLCLASS_ANIMATION_H
#define SFMLCLASS_ANIMATION_H

#include "Common.h"
#include "Utilities.h"

class Animation
{
private:
	std::string                 m_name{ "none" };
	std::vector<sf::IntRect>    m_frames;
	sf::Time                    m_timePerFrame;
	size_t                      m_currentFrame{ 0 };
	sf::Time                    m_countDown{ sf::Time::Zero };
	bool                        m_isRepeating{ true };
	bool                        m_hasEnded{ false };
	sf::Sprite                  m_sprite;


public:

	bool						m_isRotated{ false };

	Animation();
	Animation(const std::string& name, const sf::Texture& t);
	Animation(const std::string& name, const sf::Texture& t,
		std::vector<sf::IntRect> frames, sf::Time tpf, bool repeats = true, bool isRotated = false);

	void				update(sf::Time dt);
	bool				hasEnded() const;
	sf::Sprite&			getSprite();
	const std::string&	getName() const;
	const Vec2			getSize() const;

	template<typename T>
	inline void centerOrigin(T& t) {
		auto bounds = t.getLocalBounds();
		t.setOrigin((bounds.width / 2.f) + bounds.left,
			(bounds.height / 2.f) + bounds.top);
	}


	template<typename T>
	inline void centerOrigin(T* t) {
		auto bounds = t->getLocalBounds();
		t->setOrigin((bounds.width / 2.f) + bounds.left,
			(bounds.height / 2.f) + bounds.top);
	}
};


#endif //SFMLCLASS_ANIMATION_H
