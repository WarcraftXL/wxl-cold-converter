// Copyright (C) 2026 WarcraftXL
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include "Listfile.hpp"

#include <cstdlib>
#include <fstream>

namespace wxl::converter::core
{
    bool Listfile::Load(const std::string& path)
    {
        std::ifstream in(path);
        if (!in.is_open()) return false;

        std::string line;
        size_t loaded = 0;
        while (std::getline(in, line))
        {
            if (line.empty()) continue;
            const size_t sep = line.find(';');
            if (sep == std::string::npos) continue;

            char* end = nullptr;
            const uint32_t id = static_cast<uint32_t>(std::strtoul(line.c_str(), &end, 10));
            if (end != line.c_str() + sep) continue; // the id must span exactly up to the separator

            std::string filePath = line.substr(sep + 1);
            while (!filePath.empty() && (filePath.back() == '\r' || filePath.back() == '\n')) filePath.pop_back();
            if (filePath.empty()) continue;

            map_[id] = std::move(filePath);
            ++loaded;
        }
        return loaded > 0;
    }

    bool Listfile::Resolve(uint32_t fileDataId, std::string& outPath) const
    {
        const auto it = map_.find(fileDataId);
        if (it == map_.end()) return false;
        outPath = it->second;
        return true;
    }
}
