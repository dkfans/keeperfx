#ifndef RENDERER_IRENDERER_H
#define RENDERER_IRENDERER_H

// Selectable renderer backends.
enum RendererType {
    RENDERER_INVALID  = -1,
    RENDERER_AUTO     = 0,  // pick the best available backend at startup
    RENDERER_SOFTWARE = 1,  // CPU software renderer, SDL display output
};

// Backend-agnostic renderer interface. Grown as drawing migrates behind the seam.
class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual bool Init() = 0;
    virtual void Shutdown() = 0;
    virtual const char* GetName() const = 0;
    
    virtual void SetDisplayPalette(const unsigned char* rgb8) { (void)rgb8; }

    // Clear the whole display to a palette index. Default no-op.
    virtual void ClearScreen(unsigned char colour) { (void)colour; }

    // Present the drawn frame to the window (blit + flip). Default no-op.
    virtual void PresentFrame() {}
};

#endif // RENDERER_IRENDERER_H
