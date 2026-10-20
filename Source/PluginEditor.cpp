#include "PluginEditor.h"
#include "BinaryData.h"

// =====================================================================
//  palette
// =====================================================================
namespace {
const juce::Colour kBg        {0xff0b0a0c};
const juce::Colour kCream     {0xfff4e9de};
const juce::Colour kTan       {0xffd2bda8};
const juce::Colour kMuted     {0xffa79583};
const juce::Colour kDimText   {0xff8d7762};
const juce::Colour kBronze    {0xffc9925a};
const juce::Colour kBronzeHi  {0xffe8b274};
const juce::Colour kBronzeLo  {0xff6b4526};
const juce::Colour kPink      {0xffff4f9b};
const juce::Colour kPinkHi    {0xffff7ac2};
const juce::Colour kViolet    {0xffb06bff};
const juce::Colour kDarkFill  {0xff161014};
const juce::Colour kWarmFill  {0xff1b1210};
const juce::Colour kPanelFill {0xff120c0e};

void strokeBronze (juce::Graphics& g, juce::Rectangle<float> r, float radius, float thin = 1.4f)
{
    for (int pass = 0; pass < 2; ++pass)
    {
        const float alpha = pass == 0 ? 0.20f : 0.95f;
        const float thick = pass == 0 ? 5.0f : thin;
        juce::ColourGradient grad (kBronzeHi.withAlpha (alpha), r.getX(), r.getY(),
                                   kBronzeLo.withAlpha (alpha), r.getRight(), r.getBottom(), false);
        g.setGradientFill (grad);
        g.drawRoundedRectangle (r, radius, thick);
    }
}

void fillBronzeBar (juce::Graphics& g, juce::Rectangle<float> r, float radius)
{
    juce::ColourGradient grad (juce::Colour(0xffa97b45), r.getX(), r.getY(),
                               juce::Colour(0xff6e4526), r.getRight(), r.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (r, radius);
}

void strokePinkGlow (juce::Graphics& g, juce::Rectangle<float> r, float radius)
{
    g.setColour (kPink.withAlpha (0.22f));
    g.drawRoundedRectangle (r.expanded (2.0f), radius + 2.0f, 6.0f);
    g.setColour (kPink);
    g.drawRoundedRectangle (r, radius, 1.8f);
}

} // namespace

// =====================================================================
//  icons  (normalised 0..1 inside the given area)
// =====================================================================
void frDrawIcon (juce::Graphics& g, FrIcon icon, juce::Rectangle<float> a, juce::Colour c)
{
    const float x = a.getX(), y = a.getY(), w = a.getWidth(), h = a.getHeight();
    auto P = [&] (float u, float v) { return juce::Point<float> (x + u * w, y + v * h); };
    const float thick = juce::jmax (1.2f, juce::jmin (w, h) * 0.085f);
    juce::PathStrokeType stroke (thick, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
    juce::Path p;
    bool filled = false;

    switch (icon)
    {
        case FrIcon::logo:
        {
            // stylised "D"
            p.startNewSubPath (P (0.03f, 0.10f));
            p.lineTo (P (0.03f, 0.90f));
            p.startNewSubPath (P (0.03f, 0.10f));
            p.lineTo (P (0.22f, 0.10f));
            p.addCentredArc (x + 0.22f * w, y + 0.50f * h, 0.18f * w, 0.40f * h,
                             0.0f, juce::MathConstants<float>::pi * 1.5f,
                             juce::MathConstants<float>::pi * 0.5f, true);
            p.lineTo (P (0.03f, 0.90f));
            // stylised "A"
            p.startNewSubPath (P (0.50f, 0.90f));
            p.lineTo (P (0.72f, 0.10f));
            p.lineTo (P (0.94f, 0.90f));
            p.startNewSubPath (P (0.585f, 0.62f));
            p.lineTo (P (0.855f, 0.62f));
            break;
        }

        case FrIcon::save:   // floppy disk
        {
            juce::Path outer;
            outer.addRoundedRectangle (x + 0.06f * w, y + 0.06f * h, 0.88f * w, 0.88f * h, 0.10f * w);
            p = outer;
            juce::Path shutter;
            shutter.addRoundedRectangle (x + 0.30f * w, y + 0.06f * h, 0.50f * w, 0.32f * h, 0.04f * w);
            juce::Path label;
            label.addRoundedRectangle (x + 0.20f * w, y + 0.52f * h, 0.60f * w, 0.42f * h, 0.04f * w);
            g.setColour (c);
            g.strokePath (p, stroke);
            g.strokePath (shutter, stroke);
            g.strokePath (label, stroke);
            return;
        }

        case FrIcon::load:   // folder
        {
            p.startNewSubPath (P (0.06f, 0.24f));
            p.lineTo (P (0.36f, 0.24f));
            p.lineTo (P (0.46f, 0.08f));
            p.lineTo (P (0.78f, 0.08f));
            p.lineTo (P (0.88f, 0.24f));
            p.lineTo (P (0.94f, 0.24f));
            p.lineTo (P (0.94f, 0.88f));
            p.lineTo (P (0.06f, 0.88f));
            p.closeSubPath();
            break;
        }

        case FrIcon::gear:
        {
            const auto c0 = P (0.5f, 0.5f);
            const float r1 = 0.20f * juce::jmin (w, h), r2 = 0.40f * juce::jmin (w, h);
            p.addEllipse (c0.x - r1, c0.y - r1, r1 * 2, r1 * 2);
            p.addEllipse (c0.x - r2, c0.y - r2, r2 * 2, r2 * 2);
            g.setColour (c);
            g.strokePath (p, stroke);
            juce::Path teeth;
            for (int i = 0; i < 8; ++i)
            {
                const float ang = juce::MathConstants<float>::twoPi * (float) i / 8.0f;
                teeth.startNewSubPath (c0.x + std::cos (ang) * r2, c0.y + std::sin (ang) * r2);
                teeth.lineTo (c0.x + std::cos (ang) * r2 * 1.32f, c0.y + std::sin (ang) * r2 * 1.32f);
            }
            g.strokePath (teeth, stroke);
            return;
        }

        case FrIcon::room:   // isometric cube
        {
            const juce::Point<float> v[6] = { P (0.50f, 0.04f), P (0.95f, 0.27f), P (0.95f, 0.73f),
                                              P (0.50f, 0.96f), P (0.05f, 0.73f), P (0.05f, 0.27f) };
            p.startNewSubPath (v[0]);
            for (int i = 1; i < 6; ++i) p.lineTo (v[i]);
            p.closeSubPath();
            const auto c0 = P (0.50f, 0.50f);
            for (int i : { 1, 3, 5 })
            {
                p.startNewSubPath (c0);
                p.lineTo (v[i]);
            }
            break;
        }

        case FrIcon::hall:   // arch
        {
            p.startNewSubPath (P (0.18f, 0.92f));
            p.lineTo (P (0.18f, 0.42f));
            p.addCentredArc (x + 0.50f * w, y + 0.42f * h, 0.32f * w, 0.34f * h,
                             0.0f, juce::MathConstants<float>::pi,
                             juce::MathConstants<float>::twoPi, false);
            p.lineTo (P (0.82f, 0.92f));
            break;
        }

        case FrIcon::plate:  // stacked plates
        {
            for (int i = 0; i < 4; ++i)
            {
                const float v = 0.20f + 0.20f * (float) i;
                p.startNewSubPath (P (0.12f, v));
                p.lineTo (P (0.88f, v));
            }
            break;
        }

        case FrIcon::chamber:// double arch
        {
            p.startNewSubPath (P (0.12f, 0.92f));
            p.lineTo (P (0.12f, 0.42f));
            p.addCentredArc (x + 0.50f * w, y + 0.42f * h, 0.38f * w, 0.36f * h,
                             0.0f, juce::MathConstants<float>::pi,
                             juce::MathConstants<float>::twoPi, false);
            p.lineTo (P (0.88f, 0.92f));
            p.startNewSubPath (P (0.32f, 0.92f));
            p.lineTo (P (0.32f, 0.48f));
            p.addCentredArc (x + 0.50f * w, y + 0.48f * h, 0.18f * w, 0.18f * h,
                             0.0f, juce::MathConstants<float>::pi,
                             juce::MathConstants<float>::twoPi, false);
            p.lineTo (P (0.68f, 0.92f));
            break;
        }

        case FrIcon::vintage:// twin reels
        {
            const float r = 0.21f * juce::jmin (w, h);
            auto cL = P (0.30f, 0.50f), cR = P (0.70f, 0.50f);
            p.addEllipse (cL.x - r, cL.y - r, r * 2, r * 2);
            p.addEllipse (cR.x - r, cR.y - r, r * 2, r * 2);
            g.setColour (c);
            g.strokePath (p, stroke);
            const float dr = r * 0.30f;
            g.fillEllipse (cL.x - dr, cL.y - dr, dr * 2, dr * 2);
            g.fillEllipse (cR.x - dr, cR.y - dr, dr * 2, dr * 2);
            return;
        }

        case FrIcon::ambient:// sparkle
        {
            auto sparkle = [&] (juce::Point<float> c0, float r)
            {
                juce::Path s;
                s.startNewSubPath ({ c0.x, c0.y - r });
                s.quadraticTo (c0.x, c0.y, c0.x + r, c0.y);
                s.quadraticTo (c0.x, c0.y, c0.x, c0.y + r);
                s.quadraticTo (c0.x, c0.y, c0.x - r, c0.y);
                s.quadraticTo (c0.x, c0.y, c0.x, c0.y - r);
                s.closeSubPath();
                return s;
            };
            p.addPath (sparkle (P (0.46f, 0.50f), 0.38f * w));
            p.addPath (sparkle (P (0.82f, 0.20f), 0.14f * w));
            p.addPath (sparkle (P (0.18f, 0.80f), 0.11f * w));
            filled = true;
            break;
        }

        case FrIcon::search:
        {
            const auto c0 = P (0.42f, 0.42f);
            const float r = 0.27f * juce::jmin (w, h);
            p.addEllipse (c0.x - r, c0.y - r, r * 2, r * 2);
            p.startNewSubPath (c0.x + r * 0.72f, c0.y + r * 0.72f);
            p.lineTo (P (0.90f, 0.90f));
            break;
        }

        case FrIcon::shuffle:
        {
            p.startNewSubPath (P (0.08f, 0.28f));
            p.lineTo (P (0.32f, 0.28f));
            p.lineTo (P (0.55f, 0.72f));
            p.lineTo (P (0.90f, 0.72f));
            p.startNewSubPath (P (0.08f, 0.72f));
            p.lineTo (P (0.32f, 0.72f));
            p.lineTo (P (0.55f, 0.28f));
            p.lineTo (P (0.90f, 0.28f));
            g.setColour (c);
            g.strokePath (p, stroke);
            juce::Path heads;
            heads.startNewSubPath (P (0.74f, 0.58f));
            heads.lineTo (P (0.90f, 0.72f));
            heads.lineTo (P (0.74f, 0.86f));
            heads.startNewSubPath (P (0.74f, 0.14f));
            heads.lineTo (P (0.90f, 0.28f));
            heads.lineTo (P (0.74f, 0.42f));
            g.strokePath (heads, stroke);
            return;
        }

        case FrIcon::starOutline:
        case FrIcon::starFilled:
        {
            const auto c0 = P (0.50f, 0.52f);
            const float R = 0.48f * juce::jmin (w, h), r = R * 0.42f;
            for (int i = 0; i < 10; ++i)
            {
                const float ang = juce::MathConstants<float>::pi * (-0.5f + 0.2f * (float) i);
                const float rad = (i % 2 == 0) ? R : r;
                const juce::Point<float> pt (c0.x + std::cos (ang) * rad, c0.y + std::sin (ang) * rad);
                if (i == 0) p.startNewSubPath (pt); else p.lineTo (pt);
            }
            p.closeSubPath();
            filled = (icon == FrIcon::starFilled);
            break;
        }

        case FrIcon::list:
        {
            for (int i = 0; i < 3; ++i)
            {
                const float v = 0.28f + 0.22f * (float) i;
                p.startNewSubPath (P (0.28f, v));
                p.lineTo (P (0.92f, v));
            }
            g.setColour (c);
            g.strokePath (p, stroke);
            for (int i = 0; i < 3; ++i)
            {
                const float v = 0.28f + 0.22f * (float) i;
                const auto d = P (0.10f, v);
                const float dr = thick * 0.55f;
                g.fillEllipse (d.x - dr, d.y - dr, dr * 2, dr * 2);
            }
            return;
        }

        case FrIcon::prev:
        {
            p.startNewSubPath (P (0.62f, 0.18f));
            p.lineTo (P (0.30f, 0.50f));
            p.lineTo (P (0.62f, 0.82f));
            break;
        }

        case FrIcon::next:
        {
            p.startNewSubPath (P (0.38f, 0.18f));
            p.lineTo (P (0.70f, 0.50f));
            p.lineTo (P (0.38f, 0.82f));
            break;
        }
    }

    g.setColour (c);
    if (filled) g.fillPath (p); else g.strokePath (p, stroke);
}

namespace {
const char* kCategoryNames[10] = { "ALL PRESETS", "PIANO", "VOCAL", "GUITAR", "STRINGS",
                                   "DRUMS", "SYNTH / PAD", "CINEMATIC", "SPECIAL", "USER PRESETS" };

// =====================================================================
//  value formatting
// =====================================================================
juce::String formatParameterValue (int index, double value)
{
    switch (index)
    {
        case 0: return juce::String (value, 1) + " ms";
        case 1: return juce::String (value, 1) + " s";
        case 2:
        case 3:
        case 6: return juce::String (value, 1) + "%";
        case 4: return juce::String ((int) std::round (value)) + " Hz";
        case 5: return value >= 1000.0
            ? juce::String (value / 1000.0, 1) + " kHz"
            : juce::String ((int) std::round (value)) + " Hz";
        default: return juce::String (value, 1);
    }
}

// =====================================================================
//  factory bank  (24 presets, order = mock-up column order)
// =====================================================================
const FeltPreset kFactory[] = {
    { "Intimate Room",   1, {  20,  3.5f,  70, 45, 100, 12000, 35, 0 } },
    { "Close Piano",     1, {  12,  1.8f,  40, 55, 130, 11000, 28, 0 } },
    { "Warm Room",       1, {  16,  2.6f,  55, 62,  95,  8500, 30, 0 } },
    { "Bright Room",     1, {  14,  2.0f,  50, 22, 150, 18000, 26, 0 } },
    { "Studio Room",     1, {   7,  1.1f,  24, 48, 170, 14000, 20, 0 } },
    { "Wide Room",       1, {  24,  3.0f,  82, 38, 110, 15000, 33, 0 } },
    { "Concert Hall",    7, {  52,  6.8f,  96, 25,  90, 16000, 42, 1 } },
    { "Dark Hall",       7, {  48,  7.2f,  90, 80, 120,  7800, 44, 1 } },
    { "Vocal Plate",     2, {  42,  2.8f,  63, 32, 250, 14000, 26, 2 } },
    { "Warm Plate",      2, {  38,  3.2f,  60, 55, 220, 11000, 28, 2 } },
    { "Bright Plate",    2, {  24,  3.1f,  68, 12, 210, 19000, 34, 2 } },
    { "Vintage Plate",   2, {  55,  4.0f,  66, 72, 190,  7500, 32, 4 } },
    { "Chamber",         4, {  36,  2.3f,  55, 44, 230, 13000, 30, 3 } },
    { "Large Chamber",   4, {  50,  4.5f,  85, 42, 180, 12000, 38, 3 } },
    { "Ambient Space",   6, {  72,  8.0f,  95, 18, 140, 17000, 55, 5 } },
    { "Dreamy Shimmer",  6, {  65,  8.5f,  92, 40, 170, 11500, 52, 5 } },
    { "Guitar Room",     3, {  14,  1.8f,  46, 40, 180, 15000, 24, 3 } },
    { "Guitar Hall",     3, {  32,  3.6f,  72, 35, 160, 13500, 30, 1 } },
    { "Strings Hall",    4, {  33,  5.5f,  78, 60, 140, 11500, 35, 1 } },
    { "Drum Room",       5, {   5,  0.9f,  30, 36, 280, 16000, 18, 0 } },
    { "Drum Ambience",   5, {  18,  1.6f,  52, 28, 220, 15500, 30, 1 } },
    { "Synth Pad",       6, {  45,  6.2f,  88, 30, 150, 15500, 45, 5 } },
    { "Cinematic Long",  7, {  92, 11.5f, 100, 42, 100, 12500, 63, 5 } },
    { "Ethereal",        8, {  80,  9.5f,  98, 15, 130, 17500, 58, 5 } },
};
} // namespace

// =====================================================================
//  FrButton
// =====================================================================
void FrButton::paintButton (juce::Graphics& g, bool highlighted, bool down)
{
    auto r = getLocalBounds().toFloat();
    const bool selected = getToggleState();

    if (selected)                       // pink glowing character button
    {
        auto box = r.reduced (1.0f);
        g.setColour (juce::Colour(0xff2b0f24));
        g.fillRoundedRectangle (box, 12.0f);
        juce::ColourGradient wash (kPink.withAlpha (0.30f), box.getX(), box.getCentreY(),
                                   juce::Colour(0x00000000), box.getRight(), box.getCentreY(), false);
        g.setGradientFill (wash);
        g.fillRoundedRectangle (box, 12.0f);
        strokePinkGlow (g, box, 12.0f);
    }
    else
    {
        auto box = r.reduced (1.0f);
        g.setColour (down ? juce::Colour(0xff241a20) : (highlighted ? juce::Colour(0xff221822) : kDarkFill));
        g.fillRoundedRectangle (box, 12.0f);
        g.setColour (highlighted ? kBronze.withAlpha (0.65f) : juce::Colour(0xff453a42));
        g.drawRoundedRectangle (box, 12.0f, 1.2f);
    }

    const auto colour = selected ? kCream : (highlighted ? kCream : kTan);
    if (iconOnTop)
    {
        const float side = juce::jmin (r.getWidth(), r.getHeight()) * 0.40f;
        frDrawIcon (g, icon, { r.getCentreX() - side * 0.5f, r.getY() + r.getHeight() * 0.14f, side, side }, colour);
        g.setColour (colour);
        g.setFont (juce::Font (juce::FontOptions (12.0f)));
        g.drawText (getButtonText(), 0, (int) (r.getY() + r.getHeight() * 0.60f),
                    (int) r.getWidth(), (int) (r.getHeight() * 0.34f), juce::Justification::centred);
    }
    else
    {
        const float side = juce::jmin (r.getWidth(), r.getHeight()) * 0.42f;
        const auto txt = getButtonText();
        if (iconOnly || txt.isEmpty())
        {
            frDrawIcon (g, icon, { r.getCentreX() - side * 0.5f, r.getCentreY() - side * 0.5f, side, side }, colour);
            return;
        }
        g.setFont (juce::Font (juce::FontOptions (fontSize)));
        const float tw = juce::GlyphArrangement::getStringWidth (g.getCurrentFont(), txt);
        const float total = tw + side * 0.55f;
        const float left = r.getCentreX() - total * 0.5f;
        frDrawIcon (g, icon, { left, r.getCentreY() - side * 0.5f, side, side }, colour);
        g.setColour (colour);
        g.drawText (txt, (int) (left + side * 1.1f), (int) r.getY(),
                    (int) (r.getRight() - (left + side * 1.1f)), (int) r.getHeight(),
                    juce::Justification::centredLeft);
    }
}

// =====================================================================
//  FrIconComp
// =====================================================================
void FrIconComp::paint (juce::Graphics& g)
{
    frDrawIcon (g, icon, getLocalBounds().toFloat().reduced (1.0f), colour);
}

// =====================================================================
//  FrCategoryList
// =====================================================================
void FrCategoryList::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    g.setColour (kPanelFill);
    g.fillRoundedRectangle (area, 12.0f);
    strokeBronze (g, area, 12.0f);

    auto header = area.withHeight (34.0f);
    frDrawIcon (g, FrIcon::list, { header.getX() + 12.0f, header.getCentreY() - 9.0f, 18.0f, 18.0f }, kBronze);
    g.setColour (kCream);
    g.setFont (juce::Font (juce::FontOptions (15.0f)));
    g.drawText ("PRESETS", (int) (header.getX() + 38.0f), (int) header.getY(),
                (int) header.getWidth() - 46, (int) header.getHeight(), juce::Justification::centredLeft);

    const float rowH = (area.getHeight() - 40.0f) / 10.0f;
    for (int i = 0; i < 10; ++i)
    {
        auto row = juce::Rectangle<float> (area.getX() + 8.0f, area.getY() + 38.0f + i * rowH,
                                           area.getWidth() - 16.0f, rowH - 3.0f);
        if (i == selected)
        {
            fillBronzeBar (g, row, 6.0f);
            g.setColour (kCream);
        }
        else
        {
            g.setColour (kMuted);
        }
        g.setFont (juce::Font (juce::FontOptions (12.5f)));
        g.drawText (kCategoryNames[i], (int) (row.getX() + 10.0f), (int) row.getY(),
                    (int) row.getWidth() - 16, (int) row.getHeight(), juce::Justification::centredLeft);
    }
}

void FrCategoryList::mouseDown (const juce::MouseEvent& e)
{
    const float rowH = ((float) getHeight() - 40.0f) / 10.0f;
    const int idx = (int) ((e.position.y - 38.0f) / rowH);
    if (idx >= 0 && idx < 10 && e.position.y > 36.0f)
    {
        selected = idx;
        repaint();
        if (onSelect) onSelect (idx);
    }
}

// =====================================================================
//  FrPresetList
// =====================================================================
void FrPresetList::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    const int n = (int) items.size();
    const int start = page * kPage;
    const float colW = area.getWidth() / (float) kColumns;
    const float rowH = area.getHeight() / (float) kRows;

    for (int c = 0; c < kColumns; ++c)
    {
        auto col = juce::Rectangle<float> (area.getX() + c * colW + 5.0f, area.getY(),
                                           colW - 10.0f, area.getHeight());
        g.setColour (juce::Colours::white.withAlpha (0.022f));
        g.fillRoundedRectangle (col, 10.0f);

        for (int r = 0; r < kRows; ++r)
        {
            const int idx = start + c * kRows + r;
            if (idx >= n) break;
            const auto& it = items[(size_t) idx];
            auto row = col.withY (col.getY() + r * rowH).withHeight (rowH - 2.5f).reduced (3.0f, 1.0f);

            if (it.selected)
            {
                fillBronzeBar (g, row, 6.0f);
                g.setColour (kCream);
            }
            else
            {
                g.setColour (kTan);
            }
            g.setFont (juce::Font (juce::FontOptions (13.0f)));
            g.drawText (it.name, (int) (row.getX() + 10.0f), (int) row.getY(),
                        (int) row.getWidth() - 44, (int) row.getHeight(), juce::Justification::centredLeft);

            const float side = juce::jmin (row.getHeight() * 0.62f, 14.0f);
            frDrawIcon (g, it.fav ? FrIcon::starFilled : FrIcon::starOutline,
                        { row.getRight() - side - 10.0f, row.getCentreY() - side * 0.5f, side, side },
                        it.fav ? kBronzeHi : kDimText);
        }
    }
}

void FrPresetList::mouseDown (const juce::MouseEvent& e)
{
    const float colW = (float) getWidth() / (float) kColumns;
    const float rowH = (float) getHeight() / (float) kRows;
    const int c = (int) (e.position.x / colW);
    const int r = (int) (e.position.y / rowH);
    if (c < 0 || c >= kColumns || r < 0 || r >= kRows) return;

    const int idx = page * kPage + c * kRows + r;
    if (idx < 0 || idx >= (int) items.size()) return;

    const float colLeft = c * colW + 5.0f;
    const float starZone = colLeft + colW - 10.0f - 34.0f;
    if (e.position.x >= starZone)
    {
        if (onToggleStar) onToggleStar (items[(size_t) idx].flat);
    }
    else if (onSelect)
    {
        onSelect (items[(size_t) idx].flat);
    }
}

void FrPresetList::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& d)
{
    const int maxPage = (int) items.size() <= kPage ? 0 : ((int) items.size() - 1) / kPage;
    const int np = juce::jlimit (0, maxPage, page + (d.deltaY < 0.0f ? 1 : -1));
    if (np != page) { page = np; repaint(); }
}

// =====================================================================
//  knob look
// =====================================================================
void FeltRoomEditor::CopperLook::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                                   float pos, float start, float end, juce::Slider&)
{
    const auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (9.0f);
    const auto centre = bounds.getCentre();
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.385f;
    const float ringRadius = radius + 4.0f;
    const float angle = start + (end - start) * juce::jlimit (0.0f, 1.0f, pos);

    // shadow + machined rim
    g.setColour (juce::Colours::black.withAlpha (0.70f));
    g.fillEllipse (centre.x - radius - 5.0f, centre.y - radius - 2.0f,
                   (radius + 5.0f) * 2.0f, (radius + 5.0f) * 2.0f);
    g.setColour (juce::Colour(0xff51464a));
    g.drawEllipse (centre.x - ringRadius - 1.0f, centre.y - ringRadius - 1.0f,
                   (ringRadius + 1.0f) * 2.0f, (ringRadius + 1.0f) * 2.0f, 1.0f);

    // unlit track
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, ringRadius, ringRadius, 0.0f, start, end, true);
    g.setColour (juce::Colour(0xff393239));
    g.strokePath (track, juce::PathStrokeType (3.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // lit gradient arc
    const float activeEnd = start + (end - start) * juce::jlimit (0.0f, 1.0f, pos);
    constexpr int segments = 56;
    juce::Point<float> arcEnd (centre.x + std::sin (angle) * ringRadius,
                               centre.y - std::cos (angle) * ringRadius);
    for (int i = 0; i < segments; ++i)
    {
        const float t0 = (float) i / (float) segments;
        const float t1 = (float) (i + 1) / (float) segments;
        const float a0 = start + (activeEnd - start) * t0;
        const float a1 = start + (activeEnd - start) * t1;
        if (a0 >= activeEnd - 0.0001f) break;
        juce::Path arc;
        arc.addCentredArc (centre.x, centre.y, ringRadius, ringRadius, 0.0f, a0, a1, true);
        g.setColour (kPink.interpolatedWith (kViolet, t0));
        g.strokePath (arc, juce::PathStrokeType (3.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    if (pos > 0.001f)
    {
        juce::Path glowArc;
        glowArc.addCentredArc (centre.x, centre.y, ringRadius, ringRadius, 0.0f, start, activeEnd, true);
        g.setColour (kPink.withAlpha (0.12f));
        g.strokePath (glowArc, juce::PathStrokeType (8.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (kPinkHi.withAlpha (0.95f));
        g.fillEllipse (arcEnd.x - 2.2f, arcEnd.y - 2.2f, 4.4f, 4.4f);
    }

    // knob face
    auto face = juce::Rectangle<float> (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
    juce::ColourGradient faceGradient (juce::Colour(0xff2c2a2e), face.getX(), face.getY(),
                                      juce::Colour(0xff08090b), face.getRight(), face.getBottom(), false);
    g.setGradientFill (faceGradient);
    g.fillEllipse (face);
    g.setColour (juce::Colour(0xff4a444b).withAlpha (0.9f));
    g.drawEllipse (face, 1.0f);
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    g.drawEllipse (face.reduced (3.0f), 1.0f);

    // indicator
    const float pointerLength = radius * 0.72f;
    const juce::Point<float> tip (centre.x + std::sin (angle) * pointerLength,
                                  centre.y - std::cos (angle) * pointerLength);
    const juce::Point<float> base (centre.x + std::sin (angle) * (radius * 0.16f),
                                   centre.y - std::cos (angle) * (radius * 0.16f));
    g.setColour (juce::Colour(0xfff8eee8).withAlpha (0.96f));
    g.drawLine (base.x, base.y, tip.x, tip.y, 2.0f);
}

void FeltRoomEditor::CopperLook::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                               int, int, int, int, juce::ComboBox& box)
{
    auto b = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (1.0f);
    g.setColour (box.hasKeyboardFocus (true) ? juce::Colour(0xff241612) : kWarmFill);
    g.fillRoundedRectangle (b, 10.0f);
    strokeBronze (g, b, 10.0f);

    juce::Path chev;
    const float cx = b.getRight() - 18.0f, cy = b.getCentreY();
    chev.startNewSubPath (cx - 5.0f, cy - 2.5f);
    chev.lineTo (cx, cy + 3.0f);
    chev.lineTo (cx + 5.0f, cy - 2.5f);
    g.setColour (kBronze);
    g.strokePath (chev, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

// =====================================================================
//  editor
// =====================================================================
FeltRoomEditor::FeltRoomEditor (FeltRoomProcessor& p) : AudioProcessorEditor (p), proc (p)
{
    piano = juce::ImageFileFormat::loadFrom (BinaryData::piano_jpg, BinaryData::piano_jpgSize);

    buildBank();
    loadExtras();

    // ---- header buttons ----
    saveBtn = std::make_unique<FrButton> ("SAVE", FrIcon::save);
    loadBtn = std::make_unique<FrButton> ("LOAD", FrIcon::load);
    gearBtn = std::make_unique<FrButton> ("", FrIcon::gear);
    saveBtn->fontSize = 13.0f;
    loadBtn->fontSize = 13.0f;
    saveBtn->onClick = [this] { savePreset(); };
    loadBtn->onClick = [this] { loadPreset(); };
    gearBtn->onClick = [this] { showAbout(); };
    for (auto* b : { saveBtn.get(), loadBtn.get(), gearBtn.get() }) addAndMakeVisible (*b);

    // ---- character buttons ----
    const char* names[6]  = { "ROOM", "HALL", "PLATE", "CHAMBER", "VINTAGE", "AMBIENT" };
    const FrIcon icons[6] = { FrIcon::room, FrIcon::hall, FrIcon::plate,
                              FrIcon::chamber, FrIcon::vintage, FrIcon::ambient };
    for (int i = 0; i < 6; ++i)
    {
        charBtns[(size_t) i] = std::make_unique<FrButton> (names[i], icons[i], false, true);
        charBtns[(size_t) i]->onClick = [this, i]
        {
            if (auto* par = proc.params.getParameter ("MODE"))
                par->setValueNotifyingHost (par->convertTo0to1 ((float) i));
            syncWidgets();
        };
        addAndMakeVisible (*charBtns[(size_t) i]);
    }

    // ---- knobs ----
    const char* ids[] = { "PREDELAY", "DECAY", "SIZE", "DAMPING", "LOWCUT", "HIGHCUT", "MIX" };
    for (int i = 0; i < 7; ++i)
    {
        knobs[(size_t) i] = std::make_unique<juce::Slider>();
        auto& k = *knobs[(size_t) i];
        k.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        k.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        k.setLookAndFeel (&look);
        k.textFromValueFunction = [i] (double v) { return formatParameterValue (i, v); };
        k.valueFromTextFunction = [i] (const juce::String& t)
        {
            auto s = t.trim().toLowerCase().replace ("khz", "").replace ("hz", "")
                              .replace ("ms", "").replace ("s", "").replace ("%", "").trim();
            auto v = s.getDoubleValue();
            if (i == 5 && t.toLowerCase().contains ("khz")) v *= 1000.0;
            return v;
        };
        k.onValueChange = [this] { repaint(); };
        addAndMakeVisible (k);
        attachments[(size_t) i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            proc.params, ids[i], k);
    }

    // ---- preset toolbar ----
    presetBox.setTextWhenNothingSelected ("Choose a preset");
    presetBox.setLookAndFeel (&look);
    presetBox.setColour (juce::ComboBox::textColourId, kCream);
    presetBox.setColour (juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
    presetBox.setColour (juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    presetBox.setColour (juce::ComboBox::arrowColourId, kBronze);
    presetBox.setColour (juce::PopupMenu::backgroundColourId, juce::Colour(0xff1e1410));
    presetBox.setColour (juce::PopupMenu::textColourId, kCream);
    presetBox.setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff8a5c34));
    presetBox.setColour (juce::PopupMenu::highlightedTextColourId, kCream);
    presetBox.onChange = [this]
    {
        const int id = presetBox.getSelectedId();
        if (id > 0) applyFlat (id - 1);
    };
    addAndMakeVisible (presetBox);

    prevBtn  = std::make_unique<FrButton> ("", FrIcon::prev);
    nextBtn  = std::make_unique<FrButton> ("", FrIcon::next);
    randomBtn = std::make_unique<FrButton> ("RANDOM", FrIcon::shuffle);
    randomBtn->fontSize = 13.0f;
    prevBtn->onClick   = [this] { stepPreset (-1); };
    nextBtn->onClick   = [this] { stepPreset ( 1); };
    randomBtn->onClick = [this] { randomPreset(); };
    for (auto* b : { prevBtn.get(), nextBtn.get(), randomBtn.get() }) addAndMakeVisible (*b);

    searchBox.setTextToShowWhenEmpty ("Search...", kDimText);
    searchBox.setIndents (36, 1);
    searchBox.setColour (juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    searchBox.setColour (juce::TextEditor::textColourId, kCream);
    searchBox.setColour (juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    searchBox.setColour (juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    searchBox.setColour (juce::TextEditor::highlightColourId, kBronze.withAlpha (0.55f));
    searchBox.onTextChange = [this]
    {
        searchText = searchBox.getText();
        rebuildVisible();
    };
    addAndMakeVisible (searchBox);

    searchIcon = std::make_unique<FrIconComp> (FrIcon::search);
    searchIcon->colour = kDimText;
    addAndMakeVisible (*searchIcon);

    // ---- panels ----
    categories.onSelect = [this] (int c) { category = c; rebuildVisible(); };
    addAndMakeVisible (categories);

    presetList.onSelect = [this] (int flat) { applyFlat (flat); };
    presetList.onToggleStar = [this] (int flat) { toggleStar (flat); };
    addAndMakeVisible (presetList);

    proc.params.addParameterListener ("MODE", this);

    rebuildCombo();
    rebuildVisible();
    syncWidgets();

    setSize (1200, 895);
}

FeltRoomEditor::~FeltRoomEditor()
{
    proc.params.removeParameterListener ("MODE", this);
    cancelPendingUpdate();
    presetBox.setLookAndFeel (nullptr);
    for (auto& k : knobs) k->setLookAndFeel (nullptr);
}

// =====================================================================
//  bank / persistence
// =====================================================================
void FeltRoomEditor::buildBank()
{
    factory.clear();
    for (const auto& f : kFactory) factory.push_back (f);
}

void FeltRoomEditor::loadExtras()
{
    favorites.clear();
    const auto fav = proc.params.state.getProperty ("FAVORITES", juce::var()).toString();
    if (fav.isNotEmpty())
        for (const auto& t : juce::StringArray::fromTokens (fav, ";", ""))
            if (t.isNotEmpty()) favorites.add (t);

    userPresets.clear();
    const auto us = proc.params.state.getChildWithName ("USERPRESETS");
    for (int i = 0; i < us.getNumChildren(); ++i)
    {
        const auto c = us.getChild (i);
        FeltPreset up;
        up.name = c.getProperty ("name", "User").toString();
        up.category = 9;
        for (int k = 0; k < 8; ++k)
            up.p[(size_t) k] = (float) c.getProperty ("p" + juce::String (k), 0.0);
        userPresets.push_back (up);
    }
}

void FeltRoomEditor::persistExtras()
{
    auto& st = proc.params.state;
    st.setProperty ("FAVORITES", favorites.joinIntoString (";"), nullptr);

    juce::ValueTree us ("USERPRESETS");
    for (const auto& up : userPresets)
    {
        juce::ValueTree t ("PRESET");
        t.setProperty ("name", up.name, nullptr);
        for (int k = 0; k < 8; ++k)
            t.setProperty ("p" + juce::String (k), (double) up.p[(size_t) k], nullptr);
        us.appendChild (t, nullptr);
    }
    st.removeChild (st.getChildWithName ("USERPRESETS"), nullptr);
    st.appendChild (us, nullptr);
}

void FeltRoomEditor::registerUserPreset (const juce::String& name)
{
    auto v = [&] (const char* id) { return proc.params.getRawParameterValue (id)->load(); };
    FeltPreset up;
    up.name = name;
    up.category = 9;
    up.p = { v ("PREDELAY"), v ("DECAY"), v ("SIZE"), v ("DAMPING"),
             v ("LOWCUT"), v ("HIGHCUT"), v ("MIX"), v ("MODE") };

    bool replaced = false;
    for (auto& existing : userPresets)
        if (existing.name == name) { existing = up; replaced = true; break; }
    if (!replaced) userPresets.push_back (up);

    persistExtras();
    rebuildCombo();
    rebuildVisible();
}

// =====================================================================
//  view state
// =====================================================================
const FeltPreset* FeltRoomEditor::presetAt (int flat) const
{
    if (flat < 0) return nullptr;
    if (flat < (int) factory.size()) return &factory[(size_t) flat];
    const int u = flat - (int) factory.size();
    return u < (int) userPresets.size() ? &userPresets[(size_t) u] : nullptr;
}

FeltPreset* FeltRoomEditor::presetAt (int flat)
{
    return const_cast<FeltPreset*> (static_cast<const FeltRoomEditor*> (this)->presetAt (flat));
}

void FeltRoomEditor::rebuildVisible()
{
    visibleRows.clear();
    const int total = numPresets();
    for (int flat = 0; flat < total; ++flat)
    {
        const auto* pr = presetAt (flat);
        if (pr == nullptr) continue;
        const bool isUser = flat >= (int) factory.size();
        if (category == 9 && !isUser) continue;
        if (category >= 1 && category <= 8 && (isUser || pr->category != category)) continue;
        if (searchText.isNotEmpty() && !pr->name.containsIgnoreCase (searchText)) continue;

        FrPresetRow row;
        row.flat = flat;
        row.name = pr->name;
        row.fav = favorites.contains (pr->name);
        row.selected = (flat == selectedFlat);
        visibleRows.push_back (row);
    }
    presetList.items = visibleRows;
    presetList.page = 0;
    presetList.repaint();
}

void FeltRoomEditor::rebuildCombo()
{
    presetBox.clear (juce::dontSendNotification);
    const int total = numPresets();
    for (int flat = 0; flat < total; ++flat)
        if (const auto* pr = presetAt (flat))
            presetBox.addItem (pr->name, flat + 1);
    presetBox.setSelectedId (selectedFlat + 1, juce::dontSendNotification);
}

void FeltRoomEditor::applyFlat (int flat)
{
    if (auto* pr = presetAt (flat))
    {
        proc.applyPreset (pr->p);
        selectedFlat = flat;
        presetBox.setSelectedId (flat + 1, juce::dontSendNotification);
        rebuildVisible();
        repaint();
    }
}

void FeltRoomEditor::stepPreset (int delta)
{
    if (visibleRows.empty()) return;
    int pos = 0;
    for (int i = 0; i < (int) visibleRows.size(); ++i)
        if (visibleRows[(size_t) i].flat == selectedFlat) { pos = i; break; }
    pos = (pos + delta + (int) visibleRows.size()) % (int) visibleRows.size();
    applyFlat (visibleRows[(size_t) pos].flat);
}

void FeltRoomEditor::randomPreset()
{
    std::vector<int> src;
    src.reserve (visibleRows.size());
    for (const auto& r : visibleRows) src.push_back (r.flat);
    if (src.empty()) return;
    applyFlat (src[(size_t) juce::Random::getSystemRandom().nextInt ((int) src.size())]);
}

void FeltRoomEditor::toggleStar (int flat)
{
    if (const auto* pr = presetAt (flat))
    {
        if (favorites.contains (pr->name)) favorites.removeString (pr->name);
        else favorites.add (pr->name);
        persistExtras();
        rebuildVisible();
    }
}

// =====================================================================
//  file / about
// =====================================================================
void FeltRoomEditor::savePreset()
{
    chooser = std::make_unique<juce::FileChooser> (
        "Save FeltRoom preset",
        juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
            .getChildFile ("FeltRoom-Preset.xml"),
        "*.xml");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode
                            | juce::FileBrowserComponent::canSelectFiles
                            | juce::FileBrowserComponent::warnAboutOverwriting,
        [this] (const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file == juce::File {}) return;
            if (auto xml = proc.params.copyState().createXml())
            {
                xml->writeTo (file.withFileExtension ("xml"));
                registerUserPreset (file.getFileNameWithoutExtension());
            }
        });
}

void FeltRoomEditor::loadPreset()
{
    chooser = std::make_unique<juce::FileChooser> (
        "Load FeltRoom preset",
        juce::File::getSpecialLocation (juce::File::userDocumentsDirectory),
        "*.xml");
    chooser->launchAsync (juce::FileBrowserComponent::openMode
                            | juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file == juce::File {}) return;
            auto xml = juce::XmlDocument::parse (file);
            if (xml && xml->hasTagName (proc.params.state.getType()))
            {
                proc.params.replaceState (juce::ValueTree::fromXml (*xml));
                loadExtras();
                rebuildCombo();
                rebuildVisible();
                syncWidgets();
                repaint();
            }
        });
}

void FeltRoomEditor::showAbout()
{
    juce::AlertWindow::showAsync (
        juce::MessageBoxOptions()
            .withIconType (juce::MessageBoxIconType::InfoIcon)
            .withTitle ("About FeltRoom")
            .withMessage ("FeltRoom Reverb 2.0\nD.A Production\n\n"
                          "Felt Reverb — algorithmic reverb with six character modes, "
                          "24 factory presets and full preset management.")
            .withButton ("OK"),
        nullptr);
}

// =====================================================================
//  sync
// =====================================================================
void FeltRoomEditor::handleAsyncUpdate()
{
    syncWidgets();
}

void FeltRoomEditor::syncWidgets()
{
    const int mode = (int) proc.params.getRawParameterValue ("MODE")->load();
    for (int i = 0; i < 6; ++i)
        charBtns[(size_t) i]->setToggleState (i == mode, juce::dontSendNotification);
    repaint();
}

// =====================================================================
//  layout / paint
// =====================================================================
void FeltRoomEditor::resized()
{
    // header
    saveBtn->setBounds  (872, 25, 104, 42);
    loadBtn->setBounds  (984, 25, 104, 42);
    gearBtn->setBounds  (1096, 25, 44, 42);

    // character row
    for (int i = 0; i < 6; ++i)
        charBtns[(size_t) i]->setBounds (72 + i * 178, 378, 166, 66);

    // knobs
    for (int i = 0; i < 7; ++i)
        knobs[(size_t) i]->setBounds (116 + i * 162 - 56, 448, 112, 112);

    // preset panel
    categories.setBounds (28, 622, 214, 250);
    presetBox.setBounds  (258, 630, 330, 38);
    prevBtn->setBounds   (598, 630, 36, 38);
    nextBtn->setBounds   (638, 630, 36, 38);
    searchBox.setBounds  (686, 630, 322, 38);
    searchIcon->setBounds (698, 641, 16, 16);
    randomBtn->setBounds (1018, 630, 154, 38);
    presetList.setBounds (252, 678, 920, 194);
}

void FeltRoomEditor::paint (juce::Graphics& g)
{
    g.fillAll (kBg);

    // ---- hero image band ----
    auto imageArea = juce::Rectangle<int> (0, 92, getWidth(), 280);
    if (piano.isValid())
        g.drawImage (piano, imageArea.toFloat(), juce::RectanglePlacement::fillDestination);
    juce::ColourGradient topFade (juce::Colours::black.withAlpha (0.72f), 0.0f, 92.0f,
                                  juce::Colours::black.withAlpha (0.0f), 0.0f, 172.0f, false);
    g.setGradientFill (topFade);
    g.fillRect (imageArea);
    juce::ColourGradient bottomFade (juce::Colours::black.withAlpha (0.0f), 0.0f, 300.0f,
                                     juce::Colours::black.withAlpha (0.88f), 0.0f, 372.0f, false);
    g.setGradientFill (bottomFade);
    g.fillRect (imageArea);

    // ---- header ----
    frDrawIcon (g, FrIcon::logo, { 28.0f, 28.0f, 52.0f, 40.0f }, kCream);
    g.setColour (kCream);
    g.setFont (juce::Font (juce::FontOptions (20.0f)));
    g.drawText ("D\u00b7A Production", 92, 32, 220, 36, juce::Justification::centredLeft);

    g.setFont (juce::Font (juce::FontOptions (30.0f)));
    g.drawText ("F E L T R O O M", 0, 18, getWidth(), 42, juce::Justification::centred);
    g.setColour (kTan);
    g.setFont (juce::Font (juce::FontOptions (11.5f)));
    g.drawText ("F E L T   R E V E R B", 0, 62, getWidth(), 20, juce::Justification::centred);

    // ---- knob readouts ----
    const char* labels[] = { "PRE-DELAY", "DECAY", "SIZE", "DAMPING", "LOW CUT", "HIGH CUT", "MIX" };
    for (int i = 0; i < 7; ++i)
    {
        const int cx = 116 + i * 162;
        g.setColour (kCream);
        g.setFont (juce::Font (juce::FontOptions (17.0f)));
        g.drawText (formatParameterValue (i, knobs[(size_t) i]->getValue()),
                    cx - 90, 562, 180, 26, juce::Justification::centred);
        g.setColour (kMuted);
        g.setFont (juce::Font (juce::FontOptions (11.5f)));
        g.drawText (labels[i], cx - 90, 588, 180, 18, juce::Justification::centred);
    }

    // ---- preset panel frame ----
    strokeBronze (g, juce::Rectangle<float> (18.0f, 614.0f, 1164.0f, 264.0f), 16.0f);

    // ---- window edge ----
    g.setColour (kBronzeLo.withAlpha (0.55f));
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (8.0f), 12.0f, 1.0f);
}
