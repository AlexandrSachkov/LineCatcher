/*
 * This file is part of the Line Catcher distribution (https://github.com/AlexandrSachkov/LineCatcher).
 * Copyright (c) 2019 Alexandr Sachkov.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

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
