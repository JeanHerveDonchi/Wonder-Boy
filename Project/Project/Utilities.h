#ifndef BREAKOUT_UTILITIES_H
#define BREAKOUT_UTILITIES_H


#include <SFML/Graphics.hpp>
#include <iostream>


template<typename T>
inline void centerOrigin(T& t);


template<typename T>
inline void centerOrigin(T* t);


template <typename T>
inline std::ostream& operator<<(std::ostream& os, const sf::Vector2<T>& v);


template <typename T>
inline std::ostream& operator<<(std::ostream& os, const sf::Rect<T>& r);


template <typename T>
inline void displayBounds(const T& t);


#endif //BREAKOUT_UTILITIES_H
