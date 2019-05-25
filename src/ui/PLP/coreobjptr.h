#ifndef COREOBJPTR_H
#define COREOBJPTR_H

#include <memory>
#include <functional>

template<typename T>
using CoreObjPtr = std::unique_ptr<T,std::function<void(T*)>>;

#endif // COREOBJPTR_H
