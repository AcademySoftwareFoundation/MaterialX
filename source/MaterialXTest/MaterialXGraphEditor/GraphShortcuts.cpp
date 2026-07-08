//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXGraphEditor/GraphShortcuts.h>

TEST_CASE("Graph editor grouping shortcut", "[grapheditor]")
{
    GraphShortcutState state;
    state.shift = true;
    state.keyPressed = true;
    REQUIRE(isGroupSelectedNodesShortcut(state));

    state.ctrl = true;
    REQUIRE(!isGroupSelectedNodesShortcut(state));
    state.ctrl = false;

    state.alt = true;
    REQUIRE(!isGroupSelectedNodesShortcut(state));
    state.alt = false;

    state.super = true;
    REQUIRE(!isGroupSelectedNodesShortcut(state));
    state.super = false;

    state.addNodePopupOpen = true;
    REQUIRE(!isGroupSelectedNodesShortcut(state));
    state.addNodePopupOpen = false;

    state.searchPopupOpen = true;
    REQUIRE(!isGroupSelectedNodesShortcut(state));
    state.searchPopupOpen = false;

    state.fileDialogOpen = true;
    REQUIRE(!isGroupSelectedNodesShortcut(state));
}
