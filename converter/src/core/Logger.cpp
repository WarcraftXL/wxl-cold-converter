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

#include "Logger.hpp"

#include "api/Callbacks.hpp"

#include <cstdarg>
#include <cstdio>

namespace wxl::core::log
{
    namespace
    {
        bool g_infoMuted = false;

        void EmitFormatted(const char* fmt, va_list args)
        {
            char buf[1024];
            std::vsnprintf(buf, sizeof(buf), fmt, args);
            wxl::converter::api::callbacks::EmitLog(buf);
        }
    }

    void Printf(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        EmitFormatted(fmt, args);
        va_end(args);
    }

    void Info(const char* fmt, ...)
    {
        if (g_infoMuted) return;

        va_list args;
        va_start(args, fmt);
        EmitFormatted(fmt, args);
        va_end(args);
    }

    void SetInfoMuted(bool muted) { g_infoMuted = muted; }
}
