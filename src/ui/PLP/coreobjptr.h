#ifndef COREOBJPTR_H
#define COREOBJPTR_H

#include <memory>
#include <functional>
#include "Core.h"

template<typename T>
using CoreObjPtr = std::unique_ptr<T,std::function<void(T*)>>;

namespace  {
    template<typename T>
    CoreObjPtr<T> createCoreObjPtr(T* obj, PLP::CoreI* core){
        return CoreObjPtr<T>(
           obj,
           [core](T* p){
               core->release(p);
           }
        );
    }
}

#endif // COREOBJPTR_H
