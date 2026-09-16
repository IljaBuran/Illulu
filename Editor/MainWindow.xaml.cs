using System;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Interop;

namespace Editor;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();

        Loaded += MainWindow_Loaded;
        Closed += MainWindow_Closed;
        SizeChanged += MainWindow_SizeChanged;
    }

    private async void MainWindow_Loaded(object sender, RoutedEventArgs e)
    {
    }

    private async void MainWindow_SizeChanged(object sender, SizeChangedEventArgs e)
    {
    }

    private void MainWindow_Closed(object? sender, EventArgs e)
    {
    }
}