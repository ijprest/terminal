// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "KeyChord.h"

#include "KeyChord.g.cpp"

using VirtualKeyModifiers = winrt::Windows::System::VirtualKeyModifiers;

namespace winrt::Microsoft::Terminal::Control::implementation
{
    static VirtualKeyModifiers modifiersFromBooleans(bool ctrl, bool alt, bool shift, bool win)
    {
        VirtualKeyModifiers modifiers = VirtualKeyModifiers::None;
        WI_SetFlagIf(modifiers, VirtualKeyModifiers::Control, ctrl);
        WI_SetFlagIf(modifiers, VirtualKeyModifiers::Menu, alt);
        WI_SetFlagIf(modifiers, VirtualKeyModifiers::Shift, shift);
        WI_SetFlagIf(modifiers, VirtualKeyModifiers::Windows, win);
        return modifiers;
    }

    // Returns true if both arguments are not zero and equal each other.
    template<typename T>
    static bool nonZeroEquals(T lhs, T rhs)
    {
        // MSVC 17 doesn't transform boolean equations into bitwise by itself.
        // So I'm helping a bit. It's identical to: lhs && rhs && lhs == rhs
        return (lhs != 0) & (rhs != 0) & (lhs == rhs);
    }

    KeyChord::KeyChord(bool ctrl, bool alt, bool shift, bool win, int32_t vkey, int32_t scanCode) noexcept :
        KeyChord(modifiersFromBooleans(ctrl, alt, shift, win), vkey, scanCode)
    {
    }

    KeyChord::KeyChord(const VirtualKeyModifiers modifiers, int32_t vkey, int32_t scanCode) noexcept :
        _modifiers{ modifiers },
        _vkey{ vkey },
        _scanCode{ scanCode }
    {
        // ActionMap needs to identify KeyChords which should "layer" (overwrite) each other.
        // For instance win+sc(41) and win+` both specify the same KeyChord on an US keyboard layout
        // from the perspective of a user. Either of the two should correctly overwrite the other.
        // We can help ActionMap with this by ensuring that Vkey() is always valid.
        if (!_vkey)
        {
            _vkey = MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX);
        }

        assert(_vkey || _scanCode);
    }

    uint64_t KeyChord::Hash() const noexcept
    {
        auto h = uint64_t(_modifiers) << 32;
        h |= _vkey ? _vkey : _scanCode;

        // I didn't like how std::hash uses the byte-wise FNV1a for integers.
        // So I built my own std::hash with murmurhash3.
        h ^= h >> 33;
        h *= UINT64_C(0xff51afd7ed558ccd);
        h ^= h >> 33;
        h *= UINT64_C(0xc4ceb9fe1a85ec53);
        h ^= h >> 33;

        return h;
    }

    bool KeyChord::Equals(const Control::KeyChord& other) const noexcept
    {
        // Two KeyChords are equal if they have the same modifiers and either identical vkeys
        // or scancodes. But since a value of 0 indicates that the vkey/scancode isn't set,
        // we cannot simply use == to compare them. Hence we got nonZeroEquals.
        const auto otherSelf = winrt::get_self<KeyChord>(other);
        return _modifiers == otherSelf->_modifiers && (nonZeroEquals(_vkey, otherSelf->_vkey) || nonZeroEquals(_scanCode, otherSelf->_scanCode));
    }

    VirtualKeyModifiers KeyChord::Modifiers() const noexcept
    {
        return _modifiers;
    }

    void KeyChord::Modifiers(const VirtualKeyModifiers value) noexcept
    {
        _modifiers = value;
    }

    int32_t KeyChord::Vkey() const noexcept
    {
        return _vkey;
    }

    void KeyChord::Vkey(int32_t value) noexcept
    {
        _vkey = value;
    }

    int32_t KeyChord::ScanCode() const noexcept
    {
        return _scanCode;
    }

    void KeyChord::ScanCode(int32_t value) noexcept
    {
        _scanCode = value;
    }
}
