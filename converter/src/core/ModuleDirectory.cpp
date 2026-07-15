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

#include "ModuleDirectory.hpp"

#include <windows.h>

namespace wxl::converter::core
{
    std::filesystem::path ModuleDirectory()
    {
        HMODULE self = nullptr;
        // GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS: resolve the module that contains this very function,
        // i.e. this DLL -- GetModuleFileNameA(nullptr, ...) would instead return the HOST PROCESS's exe
        // (the WPF UI), which happens to share this DLL's directory today but isn't guaranteed to.
        if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                 reinterpret_cast<LPCSTR>(&ModuleDirectory), &self))
            return std::filesystem::current_path();

        char buf[MAX_PATH]{};
        const DWORD len = GetModuleFileNameA(self, buf, MAX_PATH);
        if (len == 0 || len == MAX_PATH) return std::filesystem::current_path();
        return std::filesystem::path(buf).parent_path();
    }
}
