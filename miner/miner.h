/* 
 *       Copyright (C) 2022 ElectronRock - All Rights Reserved
 */

#pragma once

#include <array>

class miner final {
public:
        miner() = default;
        ~miner() = default;
        using header_t = std::array<unsigned char, 80>;

        std::size_t do_work(const header_t& header, std::size_t complecety);
};
