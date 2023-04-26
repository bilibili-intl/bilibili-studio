#include "text_drawing_effect.h"


TextDrawingEffect::TextDrawingEffect()
    : text_color_(D2D1::ColorF(D2D1::ColorF::White)),
      underline_color_(D2D1::ColorF(D2D1::ColorF::White)),
      strikethrough_color_(D2D1::ColorF(D2D1::ColorF::White)),
      ref_count_(1) {}

TextDrawingEffect::~TextDrawingEffect() {}


STDMETHODIMP_(unsigned long) TextDrawingEffect::AddRef() {
    return InterlockedIncrement(&ref_count_);
}

STDMETHODIMP_(unsigned long) TextDrawingEffect::Release() {
    auto nc = InterlockedDecrement(&ref_count_);
    if (nc == 0) {
        delete this;
    }

    return nc;
}

STDMETHODIMP TextDrawingEffect::QueryInterface(
    REFIID riid, void** ppvObject)
{
    if (ppvObject == nullptr) {
        return E_POINTER;
    }

    if (__uuidof(TextDrawingEffect) == riid) {
        *ppvObject = this;
    } else if (__uuidof(IUnknown) == riid) {
        *ppvObject = this;
    } else {
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}