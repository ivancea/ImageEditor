#ifndef MUTEXEDIMAGE_H
#define MUTEXEDIMAGE_H

#include <functional>
#include <mutex>
#include <SFML/Graphics.hpp>

#include "Image.h"

class MutexedImage : public sf::Drawable{
    mutable std::mutex _m;
    Image* _img;
    bool _deleteOnDestroy;

public:
    MutexedImage() = delete;
    MutexedImage(const MutexedImage&) = delete;

    MutexedImage(Image* img, bool deleteOnDestroy = false){
        _img = img;
        _deleteOnDestroy = deleteOnDestroy;
    }

    ~MutexedImage(){
        if(_deleteOnDestroy)
            delete _img;
    }

    void setDeleteOnDestroy(bool deleteOnDestroy){
        _deleteOnDestroy = deleteOnDestroy;
    }
    bool getDeleteOnDestroy() const{
        return _deleteOnDestroy;
    }

    void lock(){
        _m.lock();
    }

    void unlock(){
        _m.unlock();
    }

    template <class K>
    K call(std::function< K (Image*&, void*) > func, void* data = nullptr){
        std::lock_guard<std::mutex> lg(_m);
        return func(_img, data);
    }

    int getX(){
        std::lock_guard<std::mutex> lg(_m);
        return _img->getX();
    }

    int getY(){
        std::lock_guard<std::mutex> lg(_m);
        return _img->getY();
    }

    virtual void draw(sf::RenderTarget& rt, sf::RenderStates rs) const{
        std::lock_guard<std::mutex> lg(_m);
        rt.draw(*_img);
    }
};

#endif // MUTEXEDIMAGE_H
