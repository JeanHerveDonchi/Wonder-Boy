#include "Animation.h"
#include "Utilities.h"

Animation::Animation()
{}

Animation::Animation(const std::string & name, const sf::Texture & t)
    : Animation(name, t, {}, sf::Time::Zero, true, false)
{
    m_frames = std::vector<sf::IntRect>{ sf::IntRect(0, 0, t.getSize().x, t.getSize().y) };
}

Animation::Animation(const std::string& name, 
                    const sf::Texture& t, 
                    std::vector<sf::IntRect> frames, 
                    sf::Time tpf, 
                    bool repeats, 
                    bool isRotated)
        : m_name(name)
        , m_frames(frames)
        , m_timePerFrame(tpf)
        , m_isRepeating(repeats)
        , m_countDown(tpf)
        , m_sprite(t, m_frames[0])
        , m_isRotated(isRotated)
{
    centerOrigin(m_sprite);
    if (isRotated) {
		m_sprite.setRotation(90);
	}

    std::cout << name << " tpf: " << m_timePerFrame.asMilliseconds() << "ms\n";
}

void Animation::update(sf::Time dt)
{

    m_countDown -= dt;
    if (m_countDown < sf::Time::Zero) {
        m_countDown = m_timePerFrame;
        m_currentFrame += 1;

        if (m_currentFrame == m_frames.size() && !m_isRepeating)
            return;  // on the last frame of non-repeating animaton, leave it
        else
            m_currentFrame = (m_currentFrame % m_frames.size());

        m_sprite.setTextureRect(m_frames[m_currentFrame]);
        centerOrigin(m_sprite);
    }
}


bool Animation::hasEnded() const {
    return m_hasEnded;
}


const std::string &Animation::getName() const {
    return m_name;
}

const Vec2 Animation::getSize() const {
    return Vec2(static_cast<sf::Vector2f>(m_frames[m_currentFrame].getSize()).x, static_cast<sf::Vector2f>(m_frames[m_currentFrame].getSize()).y);
}

sf::Sprite &Animation::getSprite() {
    return m_sprite;
}

