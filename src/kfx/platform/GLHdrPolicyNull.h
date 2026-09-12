#ifndef GLHDRPOLICYNULL_H
#define GLHDRPOLICYNULL_H

#include "kfx/platform/IGLHdrPolicy.h"

struct SDL_Window;

/** No Op Policy : Used for platforms that don't support HDR yet. */
class GLHdrPolicyNull : public IGLHdrPolicy {
public:
    void OnContextReady(SDL_Window* window, bool desktop_fullscreen) override;
    void OnPresent() override;
    void Shutdown() override;
};

extern GLHdrPolicyNull g_hdr_policy_null;

#endif // GLHDRPOLICYNULL_H
