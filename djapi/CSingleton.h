#include <new>

template<typename T>
class Singleton {
public : 
    static T* GetSingleton();
    static void Destroy();

protected : 
    inline explicit Singleton() { 
        assert(Singleton::instance_ == 0); 
        Singleton::instance_ = static_cast<T*>(this); 
    }
    inline ~Singleton() { 
        Singleton::instance_ = 0; 
    }

private : 
    static T* CreateInstance();
    static void ScheduleForDestruction(void (*)());
    static void DestroyInstance(T*);

private : 
    static T* instance_;

private : 
    inline explicit Singleton(Singleton const&) {}
    inline Singleton& operator=(Singleton const&) { return *this; }
};    //    end of class Singleton

template<typename T>
typename T* Singleton<T>::GetSingleton() {
    if ( Singleton::instance_ == 0 ) {
        Singleton::instance_ = CreateInstance();
        ScheduleForDestruction(Singleton::Destroy);
    }
    return Singleton::instance_;
}

template<typename T>
void Singleton<T>::Destroy() {
    if ( Singleton::instance_ != 0 ) {
        DestroyInstance(Singleton::instance_);
        Singleton::instance_ = 0;
    }
}

template<typename T>
inline typename T* Singleton<T>::CreateInstance() {
    return new T();
}

template<typename T>
inline void Singleton<T>::ScheduleForDestruction(void (*pFun)()) {
    std::atexit(pFun);
}

template<typename T>
inline void Singleton<T>::DestroyInstance(T* p) {
    delete p;
}

template<typename T>
typename T* Singleton<T>::instance_ = 0;
