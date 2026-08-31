//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

/// @file
/// Shared shortcut predicates for the MaterialX Graph Editor.

#ifndef MATERIALX_GRAPH_SHORTCUTS_H
#define MATERIALX_GRAPH_SHORTCUTS_H

/// @struct GraphShortcutState
/// Captures the UI input state needed to evaluate graph editor shortcuts.
struct GraphShortcutState
{
    bool shift = false;
    bool ctrl = false;
    bool alt = false;
    bool super = false;
    bool keyPressed = false;
    bool addNodePopupOpen = false;
    bool searchPopupOpen = false;
    bool fileDialogOpen = false;
};

/// Return true if the current shortcut state should group selected nodes.
inline bool isGroupSelectedNodesShortcut(const GraphShortcutState& state)
{
    return state.shift && state.keyPressed &&
           !state.ctrl && !state.alt && !state.super &&
           !state.addNodePopupOpen && !state.searchPopupOpen &&
           !state.fileDialogOpen;
}

#endif
