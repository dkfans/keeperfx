#pragma once
/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file RendererManager_Internal.h
 *     Internal helpers shared between RendererManager bridge files.
 *     NOT part of the public API -- include only from RendererManager.cpp
 *     and RendererBridge_*.cpp.
 */
/******************************************************************************/

class IUIRenderer;

/** The active backend's UI renderer, or nullptr if none is active. */
IUIRenderer* RendererGetActiveUIRenderer(void);
