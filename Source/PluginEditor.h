#pragma once
#include "PluginProcessor.h"
#include <array>
#include <functional>
#include <memory>
#include <vector>

// ---------- icons drawn as vector paths ----------
enum class FrIcon { save, load, gear, room, hall, plate, chamber, vintage, ambient,
                    search, shuffle, starOutline, starFilled, list, prev, next, logo };

void frDrawIcon (juce::Graphics& g, FrIcon icon, juce::Rectangle<float> area, juce::Colour colour);

// ---------- preset bank ----------
struct FeltPreset
{
    juce::String name;
    int category = 0;                 // 1=Piano ... 8=Special, 9=User
    std::array<float, 8> p {};        // predelay, decay, size, damping, lowcut, highcut, mix, mode
};

struct FrPresetRow
{
    int flat = -1;                    // index into factory+user bank
    juce::String name;
    bool fav = false;
    bool selected = false;
};

// ---------- flat rounded button with optional icon ----------
class FrButton final : public juce::Button
{
public:
    FrButton (const juce::String& text, FrIcon ic, bool iconOnlyIn = false, bool iconOnTopIn = false)
        : juce::Button (text), icon (ic), iconOnly (iconOnlyIn), iconOnTop (iconOnTopIn) {}

    void paintButton (juce::Graphics&, bool highlighted, bool down) override;

    FrIcon icon;
    bool iconOnly = false;
    bool iconOnTop = false;
    float fontSize = 12.5f;
};

// ---------- category sidebar ----------
class FrCategoryList final : public juce::Component
{
public:
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;

    std::function<void (int)> onSelect;
    int selected = 0;
};

// ---------- 3-column preset list with favourite stars ----------
class FrPresetList final : public juce::Component
{
public:
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

    std::function<void (int)> onSelect;       // flat index
    std::function<void (int)> onToggleStar;   // flat index

    std::vector<FrPresetRow> items;           // display order (filtered)
    int page = 0;

    static constexpr int kColumns = 3;
    static constexpr int kRows    = 8;
    static constexpr int kPage    = kColumns * kRows;
};

// ---------- tiny decorative icon holder ----------
class FrIconComp final : public juce::Component
{
public:
    explicit FrIconComp (FrIcon ic) : icon (ic) { setInterceptsMouseClicks (false, false); }
    void paint (juce::Graphics&) override;
    FrIcon icon;
    juce::Colour colour { 0xffd2bda8 };
};

// =====================================================================
class FeltRoomEditor final : public juce::AudioProcessorEditor,
                             private juce::AudioProcessorValueTreeState::Listener,
                             private juce::AsyncUpdater
{
public:
    explicit FeltRoomEditor (FeltRoomProcessor&);
    ~FeltRoomEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct CopperLook final : juce::LookAndFeel_V4
    {
        void drawRotarySlider (juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
        void drawComboBox (juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
    };

    // parameter listener / async updater
    void parameterChanged (const juce::String&, float) override { triggerAsyncUpdate(); }
    void handleAsyncUpdate() override;

    void buildBank();
    void loadExtras();
    void persistExtras();
    void rebuildVisible();
    void rebuildCombo();
    void toggleStar (int flat);
    void registerUserPreset (const juce::String& name);
    void applyFlat (int flat);
    void syncWidgets();
    void stepPreset (int delta);
    void randomPreset();
    void savePreset();
    void loadPreset();
    void showAbout();
    const FeltPreset* presetAt (int flat) const;
    FeltPreset* presetAt (int flat);
    int numPresets() const { return (int) factory.size() + (int) userPresets.size(); }

    FeltRoomProcessor& proc;
    CopperLook look;
    juce::Image piano;

    // header
    std::unique_ptr<FrButton> saveBtn, loadBtn, gearBtn;

    // character row
    std::array<std::unique_ptr<FrButton>, 6> charBtns;

    // knobs
    std::array<std::unique_ptr<juce::Slider>, 7> knobs;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 7> attachments;

    // preset toolbar
    juce::ComboBox presetBox;
    std::unique_ptr<FrButton> prevBtn, nextBtn, randomBtn;
    juce::TextEditor searchBox;
    std::unique_ptr<FrIconComp> searchIcon;

    // panels
    FrCategoryList categories;
    FrPresetList presetList;

    // bank + view state
    std::vector<FeltPreset> factory;
    std::vector<FeltPreset> userPresets;
    juce::StringArray favorites;
    std::vector<FrPresetRow> visibleRows;
    int category = 0;                 // 0 = all
    int selectedFlat = 0;
    juce::String searchText;

    std::unique_ptr<juce::FileChooser> chooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FeltRoomEditor)
};
