#pragma once

#include "OpenGLESPage.g.h"

namespace winrt::AxmolAppWinRT::implementation
{
    struct OpenGLESPage : OpenGLESPageT<OpenGLESPage>
    {
        OpenGLESPage()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        void ClickHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::AxmolAppWinRT::factory_implementation
{
    struct OpenGLESPage : OpenGLESPageT<OpenGLESPage, implementation::OpenGLESPage>
    {
    };
}
