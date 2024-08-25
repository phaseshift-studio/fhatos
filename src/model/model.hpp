//  FhatOS: A Distributed Operating System
//  Copyright (c) 2024 PhaseShift Studio, LLC
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#ifndef fhatos_model_hpp
#define fhatos_model_hpp

#include <fhatos.hpp>
#include FOS_PROCESS(scheduler.hpp)
#include <structure/router.hpp>


namespace fhatos {
    class Model {
    public:
        template<typename ACTOR>
        static ptr<ACTOR> deploy(const ptr<ACTOR> &actor) {
            router()->attach(actor);
            scheduler()->spawn(actor);
            return actor;
        }
    };
}
#endif
