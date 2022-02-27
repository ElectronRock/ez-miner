/* 
 *       Copyright (C) 2022 ElectronRock - All Rights Reserved
 */

#pragma once

#include <array>

template<typename Data>
class miner final {
public:
        miner() = default;
        ~miner() = default;

        template<typename Result, typename WorkFunction, typename CheckFucntion>
        Result do_work(WorkFunction&& work_function, CheckFucntion&& check_function, const Data& data){
                // todo run parrallel tasks
        }
};
