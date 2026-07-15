// Cooperative cancellation flag for an in-flight Run(). As a DLL, there is no separate process to
// kill; the engine checks this flag itself, at the scan/classify/convert loop boundaries in
// cli/Runner.cpp.
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

namespace wxl::converter::core
{
    /** @brief Clears the cancel flag. Called once at the top of Run(). */
    void ResetCancel();

    /** @brief Sets the cancel flag. Safe to call from any thread, including mid-run. */
    void RequestCancel();

    /** @brief True once RequestCancel() has been called for the current run. */
    bool IsCancelRequested();
}
