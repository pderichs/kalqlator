// KalQlator - event_dispatcher.h
// Copyright (C) 2026  pderichs
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <any>
#include <string>

#include "event_sink.h"

class EventDispatcher {
public:
    static void registerSink(EventSink* sink) { instance() = sink; }

    static void dispatch(const std::string& name, std::any payload = {}) {
        if (auto* sink = instance()) {
            sink->onEvent(name, std::move(payload));
}
    }

private:
    static EventSink*& instance() {
        static EventSink* sink = nullptr;
        return sink;
    }
};