#include "Assets.h"
#include "MusicPlayer.h"
#include <iostream>
#include <cassert>
#include <fstream>

Assets::Assets() {
}

Assets& Assets::getInstance() {
    static Assets instance; // Meyers Singleton implementation
    return instance;
}

void Assets::addFont(const std::string&fontName, const std::string&path) {
    std::unique_ptr<sf::Font> font(new sf::Font);
    if (!font->loadFromFile(path))
        throw std::runtime_error("Load failed - " + path);

    auto rc = m_fontMap.insert(std::make_pair(fontName, std::move(font)));
    if (!rc.second)
        assert(0); // big problems if insert fails

    std::cout << "Loaded font: " << path << std::endl;
}

void Assets::addSound(const std::string&soundName, const std::string&path) {
    std::unique_ptr<sf::SoundBuffer> sb(new sf::SoundBuffer);
    if (!sb->loadFromFile(path))
        throw std::runtime_error("Load failed - " + path);

    auto rc = m_soundEffectMap.insert(std::make_pair(soundName, std::move(sb)));
    if (!rc.second)
        assert(0); // big problems if insert fails

    std::cout << "Loaded sound effect: " << path << std::endl;
}

void Assets::addTexture(const std::string&textureName, const std::string&path, bool smooth) {
    m_textureMap[textureName] = sf::Texture();
    if (!m_textureMap[textureName].loadFromFile(path)) {
        std::cerr << "Could not load texture file: " << path << std::endl;
        m_textureMap.erase(textureName);
    }
    else {
        m_textureMap.at(textureName).setSmooth(smooth);
        std::cout << "Loaded texture: " << path << std::endl;
    }
}



void Assets::addAnimation(const std::string& animationName, const std::string& textureName, size_t frameCount, size_t speed)
{
    m_animationMap[animationName] = Animation(animationName, getTexture(textureName), frameCount, speed);
}

const sf::Font& Assets::getFont(const std::string&fontName) const {
    auto found = m_fontMap.find(fontName);
    assert(found != m_fontMap.end());
    return *found->second;
}


const sf::SoundBuffer& Assets::getSound(const std::string&soundName) const {
    auto found = m_soundEffectMap.find(soundName);
    assert(found != m_soundEffectMap.end());
    return *found->second;
}


const sf::Texture& Assets::getTexture(const std::string&textureName) const {
    return m_textureMap.at(textureName);
}


const Animation& Assets::getAnimation(const std::string& name) const {
    return m_animationMap.at(name);
}

void Assets::loadSounds(const std::string& path) {
    std::ifstream confFile(path);
    if (confFile.fail()) {
        std::cerr << "Open file " << path << " failed\n";
        confFile.close();
        exit(1);
    }

    std::string token{""};
    confFile >> token;
    while (confFile) {
        if (token == "Sound") {
            std::string name, path;
            confFile >> name >> path;
            addSound(name, path);
        }
        else {
            // ignore rest of line and continue
            std::string buffer;
            std::getline(confFile, buffer);
        }
        confFile >> token;
    }
    confFile.close();

}


void Assets::loadAnimations(const std::string& path) {
    // Read Config file
    std::ifstream confFile(path);
    if (confFile.fail())
    {
        std::cerr << "Open file: " << path << " failed\n";
        confFile.close();
        exit(1);
    }

    std::string token{ "" };
    confFile >> token;
    while (confFile)
    {
        if (token == "Animation")
        {
            std::string name, texture;
            size_t frames, speed;
            confFile >> name >> texture >> frames >> speed;
            addAnimation(name, texture, frames, speed);
        }
        else
        {
            // ignore rest of line and continue
            std::string buffer;
            std::getline(confFile, buffer);
        }
        confFile >> token;
    }
    confFile.close();
}


void Assets::loadFonts(const std::string&path) {
    std::ifstream confFile(path);
    if (confFile.fail()) {
        std::cerr << "Open file " << path << " failed\n";
        confFile.close();
        exit(1);
    }

    std::string token{""};
    confFile >> token;
    while (confFile) {
        if (token == "Font") {
            std::string name, path;
            confFile >> name >> path;
            addFont(name, path);
        }
        else {
            // ignore rest of line and continue
            std::string buffer;
            std::getline(confFile, buffer);
        }
        confFile >> token;
    }
    confFile.close();
}


void Assets::loadTextures(const std::string&path) {
    // Read Config file
    std::ifstream confFile(path);
    if (confFile.fail()) {
        std::cerr << "Open file: " << path << " failed\n";
        confFile.close();
        exit(1);
    }

    std::string token{""};
    confFile >> token;
    while (confFile) {
        if (token == "Texture")
        {
            std::string name, path;
            confFile >> name >> path;
            addTexture(name, path);
        }
        else {
            // ignore rest of line and continue
            std::string buffer;
            std::getline(confFile, buffer);
        }
        confFile >> token;
    }
    confFile.close();
}



void Assets::loadFromFile(const std::string path) {
    loadFonts(path);
    loadTextures(path);
    loadSounds(path);
    loadAnimations(path); 
}
