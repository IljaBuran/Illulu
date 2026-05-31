using System.Windows;
using System.Windows.Controls;

namespace Editor.Controls;

public partial class TitleBar : UserControl
{
    public TitleBar()
    {
        InitializeComponent();
    }

    private Window ParentWindow => Window.GetWindow(this)!;

    private void Minimize_Click(object sender, RoutedEventArgs e)
    {
        ParentWindow.WindowState = WindowState.Minimized;
    }

    private void Maximize_Click(object sender, RoutedEventArgs e)
    {
        ParentWindow.WindowState = ParentWindow.WindowState == WindowState.Maximized
            ? WindowState.Normal
            : WindowState.Maximized;
    }

    private void Close_Click(object sender, RoutedEventArgs e)
    {
        ParentWindow.Close();
    }
}