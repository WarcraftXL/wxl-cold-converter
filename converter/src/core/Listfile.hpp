// FileDataID -> path table loaded from a community listfile CSV (id;path per line).
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

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace wxl::converter::core
{
    class Listfile
    {
    public:
        /**
         * @brief Loads an id;path CSV (community listfile format). Existing entries are kept on failure.
         * @param path  Path to the listfile CSV.
         * @return True if the file opened and at least one row parsed.
         */
        bool Load(const std::string& path);

        /**
         * @brief Resolves a FileDataID to its path.
         * @param fileDataId  The id to resolve.
         * @param outPath     Receives the resolved path on success.
         * @return True if the id resolved.
         */
        bool Resolve(uint32_t fileDataId, std::string& outPath) const;

        /** @brief Number of loaded id->path entries. */
        size_t Size() const { return map_.size(); }

    private:
        std::unordered_map<uint32_t, std::string> map_;
    };
}
